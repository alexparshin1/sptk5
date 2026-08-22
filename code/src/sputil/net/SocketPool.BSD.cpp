/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin                             ║
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
│                                                                              │
│   As a special exception, the copyright holder gives permission to link      │
│   this library with independent modules, whether statically or               │
│   dynamically, and to distribute the resulting work under terms of your      │
│   choice, without any of the additional requirements of section 6 of the     │
│   GNU Library General Public License. An independent module is a module      │
│   which is not derived from or based on this library. If you modify this     │
│   library, you must extend this exception to your version, but you are       │
│   not obliged to do so; if you do not wish to, delete this exception         │
│   statement from your version.                                               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#include <sptk5/Printer.h>
#include <sptk5/SystemException.h>
#include <sptk5/net/SocketPool.h>

#include <cerrno>
#include <ctime>
#include <sys/event.h>
#include <unistd.h>

using SocketEvent = struct kevent;

using namespace std;
using namespace sptk;

namespace {

/**
 * @brief Pack a registration token into kqueue's opaque per-event udata pointer.
 */
void* tokenToUData(const uint64_t token)
{
    return reinterpret_cast<void*>(static_cast<uintptr_t>(token));
}

/**
 * @brief Unpack a registration token previously stored via tokenToUData().
 */
uint64_t tokenFromUData(void* udata)
{
    return static_cast<uint64_t>(reinterpret_cast<uintptr_t>(udata));
}

} // namespace

void SocketPool::open()
{
    const scoped_lock lock(m_mutex);

    if (m_pool != INVALID_SOCKET)
    {
        return;
    }

    m_pool = kqueue();

    if (m_pool == INVALID_SOCKET)
    {
        throw SystemException("Can't create kqueue");
    }
}

void SocketPool::close()
{
    const scoped_lock lock(m_mutex);

    if (m_pool != INVALID_SOCKET)
    {
        ::close(m_pool);
        m_pool = INVALID_SOCKET;
    }
}

void SocketPool::addSocket(const SocketType socketFd, const uint64_t token, const bool rearmOneShot) const
{
    // kqueue has no separate "modify" opcode: EV_ADD both registers a new filter and
    // re-arms a OneShot filter the kernel already auto-deleted after it fired, so
    // rearmOneShot needs no special handling here (unlike epoll's EPOLL_CTL_MOD).
    (void) rearmOneShot;

    auto eventFlags = static_cast<unsigned short>(EV_ADD | EV_ENABLE);
    switch (m_triggerMode)
    {
        using enum SocketPoolTriggerMode;
        case EdgeTriggered:
            eventFlags |= EV_CLEAR;
            break;
        case OneShot:
            eventFlags |= EV_ONESHOT;
            break;
        case LevelTriggered:
            break;
    }

    SocketEvent event {};
    EV_SET(&event, socketFd, EVFILT_READ, eventFlags, 0, 0, tokenToUData(token));

    if (kevent(m_pool, &event, 1, nullptr, 0, nullptr) == -1)
    {
        processError(errno, "add socket to SocketEvents");
    }
}

void SocketPool::removeSocket(const SocketType socketFd) const
{
    if (socketFd == INVALID_SOCKET)
    {
        return;
    }

    SocketEvent event {};
    EV_SET(&event, socketFd, EVFILT_READ, EV_DELETE, 0, 0, nullptr);

    // Ignore errors: a OneShot filter is auto-removed by the kernel once it fires, so
    // deleting it again (ENOENT) is expected, not a failure - mirrors epoll_ctl(DEL)
    // being best-effort in the Linux/Windows implementation.
    kevent(m_pool, &event, 1, nullptr, 0, nullptr);
}

bool SocketPool::waitForEvents(const chrono::milliseconds& timeout)
{
    m_eventsBuffer.reserve(sizeof(SocketEvent) * m_maxEvents);
    auto* events = reinterpret_cast<SocketEvent*>(m_eventsBuffer.data());

    const auto seconds = chrono::duration_cast<chrono::seconds>(timeout);
    const auto nanoseconds = chrono::duration_cast<chrono::nanoseconds>(timeout - seconds);
    const struct timespec ts
    {
        .tv_sec = static_cast<time_t>(seconds.count()),
        .tv_nsec = static_cast<long>(nanoseconds.count()),
    };

    const auto eventCount = kevent(m_pool, nullptr, 0, events, m_maxEvents, &ts);
    if (eventCount < 0)
    {
        return m_pool != INVALID_SOCKET;
    }
    m_eventsBuffer.bytes(sizeof(SocketEvent) * eventCount);

    dispatchEvents(m_eventsBuffer);

    return true;
}

void SocketPool::dispatchEvents(Buffer& eventsBuffer)
{
    auto*      events = reinterpret_cast<SocketEvent*>(eventsBuffer.data());
    const auto eventCount = eventsBuffer.size() / sizeof(SocketEvent);
    for (size_t i = 0; i < eventCount; ++i)
    {
        const auto& event = events[i];

        const SocketEventType eventType {
            .m_data = event.data > 0,
            .m_hangup = (event.flags & EV_EOF) != 0,
            .m_error = (event.flags & EV_ERROR) != 0,
        };

        onEvent(tokenFromUData(event.udata), eventType);
    }
}

void SocketPool::processError(const int error, const String& operation) const
{
    switch (error)
    {
        case EBADF:
            if (m_pool == INVALID_SOCKET)
            {
                throw SystemException("SocketPool is not open");
            }
            throw SystemException("Socket is closed");

        case EINVAL:
            throw SystemException("Invalid event");

        case ENOENT:
            // Socket is not currently registered - benign for a re-arm race.
            break;

        default:
            throw SystemException("Can't " + operation);
    }
}
