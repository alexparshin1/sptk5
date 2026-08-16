/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin. All rights reserved.       ║
║  email                alexeyp@gmail.com                                      ║
╚══════════════════════════════════════════════════════════════════════════════╝
┌──────────────────────────────────────────────────────────────────────────────┐
│   This library is free software; you can redistribute it and/or modify it    │
│   under the terms of the GNU Library General Public License as published by  │
│   the Free Software Foundation; either version 2 of the License, or (at your │
│   option) any later version.                                                 │
│                                                                              │
│   This library is distributed in the hope that it will be useful, but        │
│   WITHOUT ANY WARRANTY; without even the implied warranty of                 │
│   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library   │
│   General Public License for more details.                                   │
│                                                                              │
│   You should have received a copy of the GNU Library General Public License  │
│   along with this library; if not, write to the Free Software Foundation,    │
│   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#pragma once

/**
 * @file IoUringBackend.h
 * @brief An io_uring alternative to epoll for SocketPool.  Private to the library.
 *
 * The public header only forward-declares sptk::IoUringBackend and holds a unique_ptr to it, so
 * liburing.h never reaches a consumer of SPTK - it is optional even on Linux, and absent on the
 * other platforms.
 *
 * The model is deliberately the same one epoll gives: this reports *readiness*, and the caller
 * still does its own read().  Multishot poll stays armed across completions, so an armed socket
 * costs no submission per event.
 *
 * Threading: liburing's submission queue has a single writer by design, and here that writer is
 * the event loop thread.  arm() and disarm() are called by other threads (the acceptor, the
 * session threads), so they only append to a queue and poke an eventfd; the loop drains that queue
 * before it waits.  Without the eventfd a new connection would sit unarmed until the loop's next
 * timeout - 100 ms by default - and would not be read until then.
 */

#include <sptk5/sptk-config.h>

#include <chrono>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

#include <sptk5/net/SocketPool.h>

#ifdef HAVE_IO_URING

#include <liburing.h>
#include <poll.h>
#include <sys/eventfd.h>
#include <unistd.h>

namespace sptk {

/**
 * @brief io_uring implementation of the socket pool's event mechanism.
 */
class IoUringBackend
{
public:
    /// Reported for every completion: the registration token and what happened to it.
    using EventHandler = std::function<void(uint64_t token, SocketEventType eventType)>;

    /**
     * @brief Create a backend, or return nullptr when io_uring cannot be used here.
     *
     * Failure is expected and not an error: the kernel may be too old, `kernel.io_uring_disabled`
     * may forbid it, and a container's seccomp profile commonly denies io_uring_setup outright
     * (Docker's default profile does). The caller falls back to epoll.
     */
    static std::unique_ptr<IoUringBackend> create(size_t maxEvents, bool levelTriggered)
    {
        auto backend = std::unique_ptr<IoUringBackend>(new IoUringBackend);
        backend->m_levelTriggered = levelTriggered;
        if (!backend->initialize(maxEvents))
        {
            return nullptr;
        }
        return backend;
    }

    ~IoUringBackend()
    {
        if (m_wakeFd >= 0)
        {
            ::close(m_wakeFd);
        }
        if (m_ringReady)
        {
            io_uring_queue_exit(&m_ring);
        }
    }

    IoUringBackend(const IoUringBackend&) = delete;
    IoUringBackend& operator=(const IoUringBackend&) = delete;

    /**
     * @brief Start watching a socket. Safe to call from any thread.
     * @param socketFd          Socket to watch.
     * @param token             Per-registration token, echoed back with every completion.
     * @param oneShot           Report at most one event, as EPOLLONESHOT does.
     */
    void arm(const SocketType socketFd, const uint64_t token, const bool oneShot)
    {
        {
            const std::scoped_lock lock(m_pendingMutex);
            m_armedTokens[socketFd] = token;
            m_armedDescriptors[token] = socketFd;
        }
        enqueue(Request {.m_operation = Operation::Arm, .m_socketFd = socketFd, .m_token = token, .m_oneShot = oneShot});
    }

    /**
     * @brief Stop watching a socket. Safe to call from any thread.
     *
     * Cancels by token rather than by descriptor. IORING_ASYNC_CANCEL_ALL on a descriptor would
     * also kill a watch armed *after* this one - and callers do exactly that, removing a socket
     * inside the event handler and re-adding it a line later, which submits both in one batch.
     */
    void disarm(const SocketType socketFd)
    {
        uint64_t token = 0;
        {
            const std::scoped_lock lock(m_pendingMutex);
            const auto             armed = m_armedTokens.find(socketFd);
            if (armed == m_armedTokens.end())
            {
                return;
            }
            token = armed->second;
            m_armedTokens.erase(armed);
            m_armedDescriptors.erase(token);
        }
        enqueue(Request {.m_operation = Operation::Disarm, .m_socketFd = socketFd, .m_token = token, .m_oneShot = false});
    }

    /**
     * @brief Submit everything pending, wait for completions, and report them.
     * @param timeout           How long to wait when nothing is ready.
     * @param eventHandler      Called once per completion.
     * @return false if the ring has been shut down.
     */
    bool wait(const std::chrono::milliseconds& timeout, const EventHandler& eventHandler)
    {
        submitPending();

        __kernel_timespec waitTime {.tv_sec = timeout.count() / 1000,
                                    .tv_nsec = (timeout.count() % 1000) * 1000000};

        io_uring_cqe* firstCompletion = nullptr;
        if (const int result = io_uring_submit_and_wait_timeout(&m_ring, &firstCompletion, 1, &waitTime, nullptr);
            result < 0 && result != -ETIME && result != -EINTR)
        {
            return false;
        }

        unsigned      head = 0;
        unsigned      completionCount = 0;
        io_uring_cqe* completion = nullptr;
        io_uring_for_each_cqe(&m_ring, head, completion)
        {
            ++completionCount;
            handleCompletion(*completion, eventHandler);
        }
        io_uring_cq_advance(&m_ring, completionCount);

        return true;
    }

private:
    enum class Operation : uint8_t
    {
        Arm,
        Disarm
    };

    struct Request
    {
        Operation  m_operation;
        SocketType m_socketFd;
        uint64_t   m_token;
        bool       m_oneShot;
    };

    /// Reserved user_data values. SocketObjectPool hands out registration tokens from 1 upwards,
    /// so the top of the range cannot collide with one.
    static constexpr uint64_t WakeToken = ~uint64_t {0};
    static constexpr uint64_t CancelToken = ~uint64_t {0} - 1;

    IoUringBackend() = default;

    bool initialize(const size_t maxEvents)
    {
        // A multishot poll can complete many times per submission, so the completion queue needs
        // to be able to run ahead of the submission queue by a wide margin.
        io_uring_params parameters {};
        parameters.flags = IORING_SETUP_CQSIZE;
        parameters.cq_entries = static_cast<unsigned>(maxEvents * 4);

        if (io_uring_queue_init_params(static_cast<unsigned>(maxEvents), &m_ring, &parameters) < 0)
        {
            return false;
        }
        m_ringReady = true;

        m_wakeFd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
        if (m_wakeFd < 0)
        {
            return false;
        }

        // The wakeup channel is itself watched by the ring, so a thread that arms a socket while
        // the loop is blocked gets it submitted immediately rather than at the next timeout.
        io_uring_sqe* submission = io_uring_get_sqe(&m_ring);
        if (submission == nullptr)
        {
            return false;
        }
        io_uring_prep_poll_multishot(submission, m_wakeFd, POLLIN);
        io_uring_sqe_set_data64(submission, WakeToken);

        return io_uring_submit(&m_ring) >= 0;
    }

    void enqueue(const Request& request)
    {
        {
            const std::scoped_lock lock(m_pendingMutex);
            m_pending.push_back(request);
        }

        constexpr uint64_t one = 1;
        [[maybe_unused]] const auto written = ::write(m_wakeFd, &one, sizeof(one));
    }

    /// Turn everything queued by other threads into submissions. Runs on the event loop thread.
    void submitPending()
    {
        std::vector<Request> requests;
        requests.swap(m_rearm);   // loop-thread only, so no lock needed
        {
            const std::scoped_lock lock(m_pendingMutex);
            requests.insert(requests.end(), m_pending.cbegin(), m_pending.cend());
            m_pending.clear();
        }

        for (size_t index = 0; index < requests.size(); ++index)
        {
            const auto& request = requests[index];

            io_uring_sqe* submission = io_uring_get_sqe(&m_ring);
            if (submission == nullptr)
            {
                // Submission queue is full: flush it and try once more.
                io_uring_submit(&m_ring);
                submission = io_uring_get_sqe(&m_ring);
                if (submission == nullptr)
                {
                    // Put the rest back at the front, so they stay ahead of anything queued
                    // meanwhile - a disarm must never overtake the arm it cancels.
                    const std::scoped_lock lock(m_pendingMutex);
                    m_pending.insert(m_pending.begin(),
                                     requests.cbegin() + static_cast<ptrdiff_t>(index), requests.cend());
                    break;
                }
            }

            if (request.m_operation == Operation::Arm)
            {
                // Single-shot in both modes: level-triggered semantics come from rearm() rather
                // than from a multishot poll, which reports only fresh wakeups.
                io_uring_prep_poll_add(submission, request.m_socketFd, PollMask);
                io_uring_sqe_set_data64(submission, request.m_token);
            }
            else
            {
                io_uring_prep_cancel64(submission, request.m_token, 0);
                io_uring_sqe_set_data64(submission, CancelToken);
            }
        }
    }

    void handleCompletion(const io_uring_cqe& completion, const EventHandler& eventHandler)
    {
        const uint64_t token = completion.user_data;

        if (token == WakeToken)
        {
            // eventfd is level-triggered, so it has to be drained or it fires forever.
            uint64_t                    counter = 0;
            [[maybe_unused]] const auto bytesRead = ::read(m_wakeFd, &counter, sizeof(counter));
            return;
        }

        if (token == CancelToken || completion.res == -ECANCELED)
        {
            return;
        }

        if (completion.res < 0)
        {
            eventHandler(token, SocketEventType {.m_data = false, .m_hangup = false, .m_error = true});
            return;
        }

        const auto pollMask = static_cast<uint32_t>(completion.res);
        const SocketEventType eventType {.m_data = (pollMask & POLLIN) != 0,
                                         .m_hangup = (pollMask & (POLLHUP | POLLRDHUP)) != 0,
                                         .m_error = (pollMask & POLLERR) != 0};

        // Re-arm before the handler runs: it may remove the socket, and a re-arm afterwards would
        // resurrect a registration the caller has just dropped.
        if (m_levelTriggered && !eventType.m_hangup && !eventType.m_error)
        {
            rearm(token);
        }
        else
        {
            // Nothing will watch this registration again, so drop it. epoll gets this for free -
            // the kernel forgets a descriptor when it is closed - but here the bookkeeping is ours,
            // and a handler is entitled to just close the socket on hangup and never call
            // removeSocket(). Without this the maps grow for the life of the process.
            forget(token);
        }

        eventHandler(token, eventType);
    }

    /**
     * @brief Drop a registration's bookkeeping. Runs on the event loop thread.
     */
    void forget(const uint64_t token)
    {
        const std::scoped_lock lock(m_pendingMutex);
        const auto             armed = m_armedDescriptors.find(token);
        if (armed == m_armedDescriptors.end())
        {
            return;
        }
        // Only erase the descriptor entry if it still points at this registration: the caller may
        // already have re-added the same descriptor under a fresh token.
        if (const auto byDescriptor = m_armedTokens.find(armed->second);
            byDescriptor != m_armedTokens.end() && byDescriptor->second == token)
        {
            m_armedTokens.erase(byDescriptor);
        }
        m_armedDescriptors.erase(armed);
    }

    /**
     * @brief Watch this registration again, because io_uring's poll does not do it for us.
     *
     * A multishot poll only completes again on a fresh wakeup, so a caller that leaves data in the
     * socket - which level-triggered epoll explicitly allows - would never hear about it again. A
     * new single-shot poll re-checks the current state and completes at once if data is still
     * there, which is the semantics being replaced. `IORING_POLL_ADD_LEVEL` would do this in the
     * kernel, but it is declared and not implemented: it returns EINVAL on 7.1.
     *
     * Runs on the event loop thread, so it queues without taking the wakeup path.
     */
    void rearm(const uint64_t token)
    {
        SocketType socketFd = INVALID_SOCKET;
        {
            const std::scoped_lock lock(m_pendingMutex);
            const auto             armed = m_armedDescriptors.find(token);
            if (armed == m_armedDescriptors.end())
            {
                return;
            }
            socketFd = armed->second;
        }
        m_rearm.push_back(Request {.m_operation = Operation::Arm, .m_socketFd = socketFd, .m_token = token, .m_oneShot = false});
    }

    /// Matches the epoll mask in SocketPool: readable, peer hangup, and error.
    static constexpr uint32_t PollMask = POLLIN | POLLRDHUP | POLLERR | POLLHUP;

    io_uring             m_ring {};
    bool                 m_ringReady {false};
    int                  m_wakeFd {-1};
    std::mutex           m_pendingMutex;    ///< Protects m_pending and m_armedTokens; never held while waiting.
    std::vector<Request> m_pending;         ///< Arm/disarm requests from threads other than the loop.
    std::unordered_map<SocketType, uint64_t> m_armedTokens;      ///< Token currently watching each descriptor.
    std::unordered_map<uint64_t, SocketType> m_armedDescriptors; ///< The same registrations, by token, for rearm().
    std::vector<Request>                     m_rearm;            ///< Re-arms raised by the loop thread itself.
    bool                                     m_levelTriggered {true}; ///< Re-arm after every event.
};

} // namespace sptk

#else // HAVE_IO_URING

namespace sptk {

/// Placeholder so SocketPool can hold a unique_ptr to it on platforms without io_uring.
class IoUringBackend
{
};

} // namespace sptk

#endif // HAVE_IO_URING
