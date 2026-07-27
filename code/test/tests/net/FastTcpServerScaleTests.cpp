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

/*
 * Scaling baseline for FastTCPServer / SocketObjectPool.
 *
 * Reproduces, at SPTK level, the point-to-point MQTT load that XMQ degrades on: N publisher
 * connections each sending one small message per second, every message routed to that pair's
 * subscriber connection. In XMQ the round trip is ~300 microseconds at 10K pairs and ~3 seconds
 * at 50K pairs - a five-fold increase in connections costing four orders of magnitude in latency.
 *
 * The routing here is a plain array index, and the payload is 16 bytes, so what the test measures
 * is the reactor: SocketObjectPool's dispatch and FastTCPServer's connection handling.
 *
 * MEASURED: the reactor does not reproduce that cliff. Median round trip, 15s measurement window
 * after a 5s warmup, all with zero dropped frames and routed == published:
 *
 *   trigger mode              10K pairs / 20K conns   50K pairs / 100K conns   ratio
 *   LevelTriggered, 128        82 us (p99 229 us)      76 us (p99 1253 us)     0.92x
 *   EdgeTriggered,  32         77 us (p99 315 us)     161 us (p99 1113 us)     2.11x
 *   OneShot,        32         62 us (p99 263 us)     212 us (p99 1496 us)     3.39x
 *
 * EdgeTriggered/32 is XMQ's Linux configuration, OneShot/32 its Windows one; OneShot is the worst
 * case for SocketObjectPool, since re-arming puts a token allocation and a map erase/insert under
 * two mutexes on every message. Even there, 100K connections at 50K messages/second sustain a
 * ~200us median, flat with connection count.
 *
 * WHERE THE CLIFF ACTUALLY IS: the receive thread hand-off, covered by the second test here.
 * XMQ does not route on the reactor thread - it queues each event to a ClientSessionReceiveThread,
 * of which there are receive_threads (3, per xmq_server.conf), and a connection is pinned to one
 * of them for its lifetime. That is a fixed amount of service capacity, so it has a knee: with W
 * microseconds of work per message and R threads it saturates past R/W messages per second.
 * Adding a 100us stand-in for MQTT parsing, topic lookup and QoS 1 bookkeeping:
 *
 *   pairs   msg/s   median round trip   peak receive queue   routed/published
 *   10K     10K       184 us                    3            150162 / 150162
 *   50K     50K      3.29 s               203,908            434826 / 750481
 *
 * 17918x, reproducing XMQ's ~300us -> ~3s. 10K messages/second against 3 threads at 100us each is
 * 33% utilization; 50K needs 167%, so the queue grows without bound and a 5x load increase costs
 * four orders of magnitude. The same connections and message rate through the reactor alone stay
 * flat, so the reactor is not the constraint - the fixed-size pinned pool in front of it is.
 *
 * POOL SIZE vs QUEUE TYPE, at 50K pairs and 100us per message (SPTK_SCALE_RECEIVE_THREADS x
 * SPTK_SCALE_SHARED_QUEUE). Peak queue is the deepest single queue, so the pinned figures are
 * spread over that many queues while the shared ones are the whole backlog in one:
 *
 *   threads  queue    median round trip   peak queue   routed/published
 *   3        pinned      3.295 s            16,745         58%
 *   3        shared      3.305 s            49,983         58%
 *   8        pinned        540 us              119        100%
 *   8        shared        298 us               52        100%
 *
 * Two separate effects, and the order matters:
 *
 *  - Pool size is the capacity term and dominates. 3 threads at 100us serve 30K messages/second
 *    against 50K demanded; 8 serve 80K. Crossing back under 100% utilization is what takes the
 *    round trip from seconds to microseconds, and no queue arrangement substitutes for it - at 3
 *    threads, shared and pinned are within 0.3% of each other, because pooling capacity that is
 *    not there changes nothing.
 *  - Queue type is a scheduling term and only pays once there is spare capacity. At 8 threads,
 *    sharing the queue takes the median from 540us to 298us (1.8x) and p99 from 8015us to 1630us
 *    (4.9x): with pinning, a connection waits behind the ~6K others bound to its thread even when
 *    another thread is idle, and that shows up mostly in the tail.
 *
 * 100K MESSAGES/SECOND ON 8 CORES. 8 cores is 8e6 microseconds of CPU per second, so 100K
 * messages/second allows 80us of total server CPU per message - reactor, receive threads, send
 * threads, syscalls, application work, everything. That budget, not the thread count, is the
 * constraint at this rate: threads beyond the core count add no capacity, they only change how
 * the work is scheduled.
 *
 * Measured at 100K messages/second (50K pairs at 2/s), 8 receive threads, shared queue. Server
 * CPU counts the reactor and receive threads only, so it excludes this test's client and
 * transfers to a host where the clients are elsewhere:
 *
 *   injected work   median   p99       server CPU     CPU per message   sustained
 *   0 us             71 us    905 us   2.63 cores       26.2 us         yes
 *   10 us            99 us    934 us   3.21 cores       32.0 us         yes
 *   20 us           144 us   1231 us   3.60 cores       36.0 us         yes
 *   40 us           441 us     11.6 ms 5.00 cores       50.8 us         marginal
 *
 * So SPTK's own floor - epoll dispatch, the reads and writes, the hand-off - is about 26us of CPU
 * per message at this connection count, or 2.6 of 8 cores at 100K messages/second. That leaves
 * roughly 54us per message for application work inside the 80us budget, and less if the deployment
 * wants headroom: holding the server near 70% of the box means about 30us for application work.
 *
 * The knee is visible at 40us injected, where p99 reaches 11.6ms and the queue starts standing at
 * over a thousand entries while the median is still only 441us - the tail degrades well before the
 * median does, so median alone will not warn that a box is running out of room.
 *
 * These tests are DISABLED_ by default - they take minutes and several GB. To run:
 *
 *   ./test/sptk_unit_tests --gtest_also_run_disabled_tests --gtest_filter='FastTcpServerScale*'
 *
 * Host requirements (this development host already satisfies them):
 *   - ulimit -n >= 300000       (the large scenario holds ~200K sockets: 100K client + 100K server)
 *   - ~4 GB free RAM
 *
 * Ephemeral ports: the large scenario opens 100K client sockets, more than the ~64K ports a single
 * source address has. Clients are therefore spread over several loopback source addresses (all of
 * 127.0.0.0/8 is local on Linux, so this needs no host configuration), keeping each address well
 * under 40K ports in use - past roughly 80% of a range the kernel's port search slows down sharply
 * and would show up as latency that has nothing to do with the server.
 *
 * After a full run the host carries ~100K sockets in TIME_WAIT for a minute or so. Other network
 * tests will fail to connect until that drains, so leave a gap before running anything else:
 *
 *   until [ "$(ss -tan | grep -c TIME-WAIT)" -lt 2000 ]; do sleep 5; done
 */

#include <gtest/gtest.h>
#include <sptk5/cutils>
#include <sptk5/net/FastTCPServer.h>
#include <sptk5/net/SocketEvents.h>
#include <sptk5/threads/SynchronizedQueue.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstring>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#ifndef _WIN32
#include <netinet/tcp.h>
#include <sys/syscall.h>
#include <unistd.h>
#endif

using namespace std;
using namespace sptk;

namespace {

/**
 * @brief Wire frame. Fixed size, so framing never depends on TCP segmentation.
 */
struct Frame
{
    uint32_t pairId {0};      ///< Pair this frame belongs to.
    uint32_t role {0};        ///< Role of the sender, on the registration frame.
    uint64_t timestampNs {0}; ///< steady_clock at publish time, carried through to the subscriber.
};

constexpr size_t frameSize = sizeof(Frame);
static_assert(frameSize == 16, "Frame is expected to be 16 bytes, matching the XMQ payload size");

constexpr uint32_t rolePublisher = 0;
constexpr uint32_t roleSubscriber = 1;

/**
 * @brief Scenario shape. The two sizes are the XMQ data points.
 */
constexpr size_t smallPairCount = 10000; ///< 20K connections.
constexpr size_t largePairCount = 50000; ///< 100K connections.

constexpr uint16_t smallScenarioPort = 12500;
constexpr uint16_t largeScenarioPort = 12501;

/**
 * @brief Loopback source addresses the client sockets are spread over.
 *
 * 100K sockets over 8 addresses is 12.5K ports each, comfortably below the ~40K at which the
 * kernel's ephemeral port search degrades.
 */
const vector<String> sourceAddresses {"127.0.0.1", "127.0.0.2", "127.0.0.3", "127.0.0.4",
                                      "127.0.0.5", "127.0.0.6", "127.0.0.7", "127.0.0.8"};

/**
 * @brief Granularity at which each publish interval is spread out.
 *
 * Real publishers are independently phased, so at N publishers and one message each per second the
 * server sees roughly N/1000 arrivals per millisecond, continuously. Slicing coarser than that
 * turns the load into periodic bursts whose queuing delay grows with N, which reads as a latency
 * cliff that is purely an artifact of the harness.
 */
constexpr auto publishSlice = chrono::milliseconds(1);

/**
 * @brief Read a size_t from the environment, or return the default.
 *
 * Lets the baseline be re-pointed at other loads (rate sweeps, larger pair counts) without a
 * rebuild - the ratio this test asserts is only one point of a curve worth exploring.
 */
size_t envSize(const char* name, const size_t defaultValue)
{
    const auto* text = getenv(name);
    if (text == nullptr || *text == 0)
    {
        return defaultValue;
    }
    return static_cast<size_t>(strtoull(text, nullptr, 10));
}

/**
 * @brief Reactor trigger mode to run the server under.
 *
 * Defaults to what XMQ uses on Linux. The mode decides how much of SocketObjectPool sits on the
 * per-message path: LevelTriggered and EdgeTriggered only dispatch, while OneShot additionally
 * re-arms every socket after every message.
 */
SocketPoolTriggerMode triggerModeFromEnv()
{
    const auto* text = getenv("SPTK_SCALE_TRIGGER");
    const String mode = text != nullptr ? String(text) : String("edge");

    if (mode == "level")
    {
        return SocketPoolTriggerMode::LevelTriggered;
    }
    if (mode == "oneshot")
    {
        return SocketPoolTriggerMode::OneShot;
    }
    return SocketPoolTriggerMode::EdgeTriggered;
}

String triggerModeName(const SocketPoolTriggerMode mode)
{
    switch (mode)
    {
        using enum SocketPoolTriggerMode;
        case LevelTriggered:
            return "LevelTriggered";
        case EdgeTriggered:
            return "EdgeTriggered";
        case OneShot:
            return "OneShot";
    }
    return "unknown";
}

constexpr size_t connectThreadCount = 8; ///< Threads opening client connections.
constexpr size_t publishThreadCount = 4; ///< Threads driving the publishers.
constexpr size_t clientPoolCount = 8;    ///< Client-side reactors, see runScenario().

uint64_t nowNs()
{
    return static_cast<uint64_t>(
        chrono::duration_cast<chrono::nanoseconds>(chrono::steady_clock::now().time_since_epoch()).count());
}

/**
 * @brief Server connection carrying its own routing state.
 *
 * Kept in the connection rather than in a map keyed by connection, so the reactor's per-event work
 * stays free of a lookup that would grow with the connection count and blur what is being measured.
 */
class PairConnection : public ServerConnection
{
public:
    PairConnection(const Type type, const sockaddr_in* peer)
        : ServerConnection(type, peer)
    {
    }

    array<uint8_t, frameSize> m_partial {};     ///< Incomplete frame carried to the next event.
    size_t                    m_partialSize {0};
    uint32_t                  m_pairId {0};
    bool                      m_registered {false}; ///< First frame received (the registration frame).
    size_t                    m_receiveThread {0}; ///< Receive thread this connection is pinned to.

    /**
     * @brief Set while this connection is queued for, or being handled by, a receive thread.
     *
     * Pinning serializes a connection's events for free, because they all land on the same thread.
     * A shared queue does not: two workers could otherwise drain the same socket at once and
     * shred m_partial. This flag keeps a connection to one worker at a time in both arrangements,
     * so the queue type stays the only difference between them.
     */
    atomic_bool m_queued {false};
};

/**
 * @brief CPU time a thread has used, in clock ticks, or 0 where that isn't available.
 *
 * Latency alone does not transfer between hosts, but CPU per message does: it is what decides
 * whether a target box has the cores for a given message rate. Reading the server's own threads
 * separates its cost from this test's client, which shares the same machine.
 */
size_t threadCpuTicks(const long tid)
{
#ifndef _WIN32
    const auto  path = "/proc/self/task/" + to_string(tid) + "/stat";
    FILE* const stat = fopen(path.c_str(), "r");
    if (stat == nullptr)
    {
        return 0;
    }
    // utime and stime are fields 14 and 15, after a comm field that may itself contain spaces -
    // so parse from the closing parenthesis rather than counting fields from the start.
    array<char, 1024> line {};
    const auto        read = fread(line.data(), 1, line.size() - 1, stat);
    fclose(stat);
    if (read == 0)
    {
        return 0;
    }

    const char* cursor = strrchr(line.data(), ')');
    if (cursor == nullptr)
    {
        return 0;
    }
    ++cursor;

    unsigned long long utime = 0;
    unsigned long long stime = 0;
    // From after comm: state is field 3, so utime is the 12th value and stime the 13th.
    if (sscanf(cursor, " %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %llu %llu", &utime, &stime) != 2)
    {
        return 0;
    }
    return static_cast<size_t>(utime + stime);
#else
    (void) tid;
    return 0;
#endif
}

long currentThreadId()
{
#ifndef _WIN32
    return static_cast<long>(syscall(SYS_gettid));
#else
    return 0;
#endif
}

/**
 * @brief Burn CPU for approximately the requested time.
 *
 * Stands in for the per-message work this test does not do - MQTT parsing, topic tree lookup,
 * QoS 1 inflight bookkeeping - so the hand-off can be driven to saturation on purpose.
 */
void simulateWork(const size_t microseconds)
{
    if (microseconds == 0)
    {
        return;
    }
    const auto until = chrono::steady_clock::now() + chrono::microseconds(microseconds);
    while (chrono::steady_clock::now() < until)
    {
        // Spin: this models CPU-bound work, which is what parsing and lookups are.
    }
}

/**
 * @brief Routes every frame from a pair's publisher to that pair's subscriber.
 *
 * All callbacks run on the single reactor thread of FastTCPServer's SocketEvents, so the routing
 * table and the read buffer need no locking; only the counters the test thread reads are atomic.
 */
class PointToPointServer : public FastTCPServer
{
public:
    PointToPointServer(const string& name, const size_t pairCount, const SocketPoolTriggerMode triggerMode,
                       const size_t maxEvents, const size_t receiveThreadCount, const size_t workMicroseconds,
                       const bool sharedQueue)
        : FastTCPServer(name, nullptr, triggerMode, maxEvents)
        , m_subscribers(pairCount)
        , m_receiveThreadCount(receiveThreadCount)
        , m_workMicroseconds(workMicroseconds)
    {
        // receiveThreadCount == 0 keeps the routing on the reactor thread.
        if (receiveThreadCount > 0)
        {
            m_receivePool = make_unique<ReceivePool>(*this, receiveThreadCount, sharedQueue);
        }
    }

    ~PointToPointServer() override
    {
        stop();
        // Only safe once stop() has joined the reactor, so nothing can queue any more work.
        m_receivePool.reset();
    }

    /**
     * @return the longest receive queue right now, 0 when routing on the reactor thread.
     */
    [[nodiscard]] size_t maxQueueLength() const
    {
        return m_receivePool ? m_receivePool->maxQueueLength() : 0;
    }

    /**
     * @return CPU ticks used by the server's own threads - the reactor plus any receive threads.
     *
     * Excludes this test's client, which shares the machine. On a real deployment the clients are
     * elsewhere, so this is the number that says whether a given box has the cores for a rate.
     */
    [[nodiscard]] size_t serverCpuTicks() const
    {
        auto ticks = m_receivePool ? m_receivePool->cpuTicks() : 0;
        if (const auto reactorTid = m_reactorTid.load(memory_order_relaxed); reactorTid != 0)
        {
            ticks += threadCpuTicks(reactorTid);
        }
        return ticks;
    }

    SServerConnection createConnection(const ServerConnection::Type connectionType, const SocketType connectionSocket,
                                       const sockaddr_in* peer) override
    {
        const STCPSocket socket = createConnectionSocket(connectionType, connectionSocket);

        auto connection = make_shared<PairConnection>(connectionType, peer);
        connection->setSocket(socket);
        if (m_receiveThreadCount > 0)
        {
            connection->m_receiveThread = m_nextReceiveThread++ % m_receiveThreadCount;
        }

        return connection;
    }

    [[nodiscard]] size_t registrations() const
    {
        return m_registrations.load(memory_order_relaxed);
    }

    [[nodiscard]] size_t routed() const
    {
        return m_routed.load(memory_order_relaxed);
    }

    [[nodiscard]] size_t dropped() const
    {
        return m_dropped.load(memory_order_relaxed);
    }

protected:
    void socketEventCallback(const shared_ptr<ServerConnection>& connection, const SocketEventType eventType) override
    {
        if (m_reactorTid.load(memory_order_relaxed) == 0)
        {
            // First event: this is the reactor thread, and SocketEvents runs exactly one.
            m_reactorTid.store(currentThreadId(), memory_order_relaxed);
        }

        if (eventType.m_hangup || eventType.m_error)
        {
            closeConnection(connection);
            return;
        }

        if (!eventType.m_data)
        {
            return;
        }

        if (m_receivePool)
        {
            // Hand off to the receive threads, as XMQ does, and go straight back to the poll.
            m_receivePool->queue(connection, static_cast<PairConnection&>(*connection).m_receiveThread);
            return;
        }

        processConnection(connection);
    }

public:
    /**
     * @brief Drain a readable connection and route whatever whole frames it yielded.
     *
     * Runs either on the reactor thread or on the connection's pinned receive thread; a given
     * connection is only ever handled by one of them at a time.
     */
    void processConnection(const shared_ptr<ServerConnection>& connection)
    {
        auto&      pairConnection = static_cast<PairConnection&>(*connection);
        const auto socket = connection->getSocket();

        // Whichever thread is draining gets its own scratch buffer.
        static thread_local vector<uint8_t> readBuffer(64 * 1024);

        // Drain to empty, re-checking after every read: under EdgeTriggered a socket left with
        // unread bytes may never be reported again, and the pair would simply go silent.
        for (auto available = socket->socketBytes(); available > 0; available = socket->socketBytes())
        {
            // Socket::read() is a single recv() and may stop mid-frame, so start from whatever the
            // previous event left over and carry any new remainder forward.
            const auto carried = pairConnection.m_partialSize;
            memcpy(readBuffer.data(), pairConnection.m_partial.data(), carried);

            const auto wanted = min(available, readBuffer.size() - carried);
            const auto bytes = socket->read(readBuffer.data() + carried, wanted);
            if (bytes == 0)
            {
                break; // Peer closed the connection.
            }

            const auto total = carried + bytes;
            size_t     offset = 0;
            while (total - offset >= frameSize)
            {
                Frame frame;
                memcpy(&frame, readBuffer.data() + offset, frameSize);
                offset += frameSize;
                route(pairConnection, frame);
            }

            pairConnection.m_partialSize = total - offset;
            memcpy(pairConnection.m_partial.data(), readBuffer.data() + offset, pairConnection.m_partialSize);
        }

        if (getTriggerMode() == SocketPoolTriggerMode::OneShot)
        {
            // OneShot disarmed this socket when the event fired, so every message costs a re-arm -
            // the same path XMQ takes on Windows, and the one that puts SocketObjectPool's
            // bookkeeping on the per-message hot path rather than only on connect/disconnect.
            watchConnection(connection, true);
        }
    }

private:
    /**
     * @brief The receive threads sitting between the reactor and the routing.
     *
     * Two arrangements, differing only in how many queues the same number of workers draw from:
     *
     *  - pinned: one queue per thread, and a connection is bound to one of them for its lifetime.
     *    This is XMQ's ClientSessionReceiveThread. A connection's messages wait behind every other
     *    connection that happened to land on the same thread, and an idle thread cannot help a
     *    backed-up one.
     *  - shared: one queue, all threads drawing from it. Any free thread takes the next connection,
     *    so capacity is pooled instead of partitioned.
     */
    class ReceivePool
    {
    public:
        ReceivePool(PointToPointServer& server, const size_t threadCount, const bool shared)
            : m_shared(shared)
        {
            for (size_t i = 0; i < (shared ? 1 : threadCount); ++i)
            {
                m_queues.push_back(make_unique<Queue>());
            }
            for (size_t i = 0; i < threadCount; ++i)
            {
                auto& queue = *m_queues[shared ? 0 : i];
                m_threads.emplace_back([this, &server, &queue] { run(server, queue); });
            }
        }

        ~ReceivePool()
        {
            terminate();
        }

        void queue(const shared_ptr<ServerConnection>& connection, const size_t pinnedIndex)
        {
            auto& pairConnection = static_cast<PairConnection&>(*connection);
            if (pairConnection.m_queued.exchange(true))
            {
                return; // Already queued or in flight; whoever holds it will drain what just arrived.
            }
            m_queues[m_shared ? 0 : pinnedIndex % m_queues.size()]->push_back(connection);
        }

        [[nodiscard]] size_t maxQueueLength() const
        {
            size_t longest = 0;
            for (const auto& queue: m_queues)
            {
                longest = max(longest, queue->size());
            }
            return longest;
        }

        void terminate()
        {
            m_terminated = true;
            for (auto& queue: m_queues)
            {
                // Drop whatever is still queued rather than working it off: a saturated run leaves
                // hundreds of thousands of entries, every one of which would just fail against an
                // already-closed peer.
                queue->clear();
                for (size_t i = 0; i < m_threads.size(); ++i)
                {
                    queue->wakeup();
                }
            }
            for (auto& workerThread: m_threads)
            {
                if (workerThread.joinable())
                {
                    workerThread.join();
                }
            }
            m_threads.clear();
        }

    private:
        using Queue = SynchronizedQueue<shared_ptr<ServerConnection>>;

        void run(PointToPointServer& server, Queue& queue)
        {
            {
                const scoped_lock lock(m_tidsMutex);
                m_tids.push_back(currentThreadId());
            }

            shared_ptr<ServerConnection> connection;
            while (!m_terminated)
            {
                if (!queue.pop_front(connection, chrono::milliseconds(100)))
                {
                    continue;
                }

                auto& pairConnection = static_cast<PairConnection&>(*connection);
                try
                {
                    server.processConnection(connection);
                }
                catch (const Exception&)
                {
                    // A peer that closed under us, typically during shutdown. On the reactor
                    // thread SocketEvents swallows this; here it would reach the thread's entry
                    // point and abort the process.
                }
                pairConnection.m_queued.store(false);

                // Anything that arrived while this connection was in flight had its event
                // suppressed by the flag, so re-queue it rather than waiting for the next one.
                try
                {
                    if (connection->getSocket()->socketBytes() > 0)
                    {
                        this->queue(connection, pairConnection.m_receiveThread);
                    }
                }
                catch (const Exception&)
                {
                    // Socket already closed.
                }
            }
        }

    public:
        /**
         * @return combined CPU ticks used by every receive thread.
         */
        [[nodiscard]] size_t cpuTicks() const
        {
            const scoped_lock lock(m_tidsMutex);
            size_t            ticks = 0;
            for (const auto tid: m_tids)
            {
                ticks += threadCpuTicks(tid);
            }
            return ticks;
        }

    private:
        // Held by pointer: SynchronizedQueue is not movable, so the vector must not relocate them.
        vector<unique_ptr<Queue>> m_queues;
        vector<thread>            m_threads;
        bool                      m_shared;
        atomic_bool               m_terminated {false};
        mutable mutex             m_tidsMutex;
        vector<long>              m_tids; ///< Worker thread ids, for CPU accounting.
    };

    void route(PairConnection& connection, const Frame& frame)
    {
        if (!connection.m_registered)
        {
            // First frame on a connection registers it as the pair's publisher or subscriber.
            connection.m_registered = true;
            connection.m_pairId = frame.pairId;
            if (frame.role == roleSubscriber && frame.pairId < m_subscribers.size())
            {
                m_subscribers[frame.pairId] = connection.getSocket();
            }
            m_registrations.fetch_add(1, memory_order_relaxed);
            return;
        }

        if (frame.pairId >= m_subscribers.size())
        {
            return;
        }

        const auto& subscriber = m_subscribers[frame.pairId];
        if (!subscriber)
        {
            return;
        }

        // Stands in for MQTT parsing, topic lookup and QoS 1 bookkeeping.
        simulateWork(m_workMicroseconds);

        try
        {
            subscriber->write(bit_cast<const uint8_t*>(&frame), frameSize);
            m_routed.fetch_add(1, memory_order_relaxed);
        }
        catch (const Exception&)
        {
            // Subscriber's send buffer is full: a real broker would queue. Count and move on -
            // the frames that do get through are still timed correctly.
            m_dropped.fetch_add(1, memory_order_relaxed);
        }
    }

    vector<STCPSocket>           m_subscribers; ///< Subscriber socket per pair id.
    unique_ptr<ReceivePool>      m_receivePool; ///< Null when routing on the reactor thread.
    size_t                       m_receiveThreadCount {0};
    size_t                       m_nextReceiveThread {0}; ///< Listener thread only.
    size_t                       m_workMicroseconds {0};
    atomic<size_t>                     m_registrations {0};
    atomic<size_t>                     m_routed {0};
    atomic<size_t>                     m_dropped {0};
    atomic<long>                       m_reactorTid {0}; ///< For CPU accounting.
};

/**
 * @brief Client subscriber socket, carrying its own frame reassembly state.
 */
class SubscriberSocket : public TCPSocket
{
public:
    array<uint8_t, frameSize> m_partial {};
    size_t                    m_partialSize {0};
};

/**
 * @brief One client-side reactor and the samples it collected.
 */
struct ClientPool
{
    vector<uint8_t>                  readBuffer = vector<uint8_t>(64 * 1024);
    vector<uint64_t>                 samplesNs;
    shared_ptr<SocketEvents<Socket>> events;
};

struct LatencyStats
{
    size_t samples {0};
    double medianUs {0};
    double meanUs {0};
    double p99Us {0};
    double maxUs {0};
    size_t routed {0};
    size_t dropped {0};
    size_t published {0};
    size_t peakQueue {0}; ///< Deepest receive queue seen while measuring.
};

/**
 * @brief What a scenario runs as. Defaults come from the environment; tests override what they
 *        are specifically about.
 */
struct ScenarioConfig
{
    SocketPoolTriggerMode triggerMode {SocketPoolTriggerMode::EdgeTriggered};
    size_t                maxEvents {32};
    size_t                publishesPerSecond {1};
    size_t                receiveThreadCount {0}; ///< 0 routes on the reactor thread.
    size_t                workMicroseconds {0};   ///< Simulated per-message parsing/lookup cost.
    bool                  sharedQueue {false};    ///< One queue for all receive threads, or one each.
    chrono::seconds       warmup {5};
    chrono::seconds       measure {10};
};

ScenarioConfig configFromEnv()
{
    return ScenarioConfig {.triggerMode = triggerModeFromEnv(),
                           .maxEvents = envSize("SPTK_SCALE_MAX_EVENTS", 32),
                           .publishesPerSecond = envSize("SPTK_SCALE_RATE", 1),
                           .receiveThreadCount = envSize("SPTK_SCALE_RECEIVE_THREADS", 0),
                           .workMicroseconds = envSize("SPTK_SCALE_WORK_US", 0),
                           .sharedQueue = envSize("SPTK_SCALE_SHARED_QUEUE", 0) != 0,
                           .warmup = chrono::seconds(envSize("SPTK_SCALE_WARMUP_SEC", 5)),
                           .measure = chrono::seconds(envSize("SPTK_SCALE_MEASURE_SEC", 10))};
}

LatencyStats summarize(vector<uint64_t> samplesNs)
{
    LatencyStats stats;
    stats.samples = samplesNs.size();
    if (samplesNs.empty())
    {
        return stats;
    }

    ranges::sort(samplesNs);

    const auto toUs = [](const uint64_t ns)
    {
        return static_cast<double>(ns) / 1000.0;
    };

    stats.medianUs = toUs(samplesNs[samplesNs.size() / 2]);
    stats.p99Us = toUs(samplesNs[static_cast<size_t>(static_cast<double>(samplesNs.size()) * 0.99)]);
    stats.maxUs = toUs(samplesNs.back());

    long double total = 0;
    for (const auto sample: samplesNs)
    {
        total += static_cast<long double>(sample);
    }
    stats.meanUs = static_cast<double>(total / static_cast<long double>(samplesNs.size())) / 1000.0;

    return stats;
}

/**
 * @brief Run one scenario end to end and return its round trip latency.
 * @param pairCount         Number of publisher/subscriber pairs.
 * @param port              Listener port for this scenario.
 */
LatencyStats runScenario(const size_t pairCount, const uint16_t port, const ScenarioConfig& config)
{
    const auto publishesPerSecond = config.publishesPerSecond;
    const auto warmupDuration = config.warmup;
    const auto measureDuration = config.measure;
    const auto publishInterval = chrono::milliseconds(1000 / max<size_t>(publishesPerSecond, 1));
    const auto triggerMode = config.triggerMode;
    const auto maxEvents = config.maxEvents;
    const auto receiveThreadCount = config.receiveThreadCount;
    const auto workMicroseconds = config.workMicroseconds;
    const auto sharedQueue = config.sharedQueue;

    PointToPointServer server("P2P-" + to_string(pairCount), pairCount, triggerMode, maxEvents, receiveThreadCount,
                              workMicroseconds, sharedQueue);
    server.addListener(ServerConnection::Type::TCP, Host("127.0.0.1", port));

    atomic<bool> collecting {false};

    // The client's receive path is sharded over several reactors, while the server under test has
    // the single reactor FastTCPServer creates. Without that the harness would saturate alongside
    // the server and the measurement could not tell the two apart.
    vector<ClientPool> pools(clientPoolCount);
    for (auto& pool: pools)
    {
        auto* poolPtr = &pool;
        pool.events = make_shared<SocketEvents<Socket>>(
            "client-pool",
            [poolPtr, &collecting](const weak_ptr<Socket>& weakSocket, const SocketEventType eventType)
            {
                if (!eventType.m_data)
                {
                    return;
                }
                const auto socket = weakSocket.lock();
                if (!socket)
                {
                    return;
                }

                auto& subscriber = static_cast<SubscriberSocket&>(*socket);
                auto  available = socket->socketBytes();
                while (available > 0)
                {
                    const auto carried = subscriber.m_partialSize;
                    memcpy(poolPtr->readBuffer.data(), subscriber.m_partial.data(), carried);

                    const auto wanted = min(available, poolPtr->readBuffer.size() - carried);
                    const auto bytes = socket->read(poolPtr->readBuffer.data() + carried, wanted);
                    if (bytes == 0)
                    {
                        break;
                    }
                    available -= bytes;

                    const auto arrived = nowNs();
                    const auto total = carried + bytes;
                    size_t     offset = 0;
                    while (total - offset >= frameSize)
                    {
                        Frame frame;
                        memcpy(&frame, poolPtr->readBuffer.data() + offset, frameSize);
                        offset += frameSize;
                        if (collecting.load(memory_order_relaxed) && arrived > frame.timestampNs)
                        {
                            poolPtr->samplesNs.push_back(arrived - frame.timestampNs);
                        }
                    }

                    subscriber.m_partialSize = total - offset;
                    memcpy(subscriber.m_partial.data(), poolPtr->readBuffer.data() + offset, subscriber.m_partialSize);
                }
            });
    }

    vector<shared_ptr<SubscriberSocket>> subscribers(pairCount);
    vector<shared_ptr<TCPSocket>>        publishers(pairCount);

    // Open both connections of every pair, registering each with the server as it goes.
    const auto connectRange = [&](const size_t from, const size_t to)
    {
        for (auto pairId = from; pairId < to; ++pairId)
        {
            const auto& sourceAddress = sourceAddresses[pairId % sourceAddresses.size()];
            const Host  serverHost("127.0.0.1", port);

            auto subscriber = make_shared<SubscriberSocket>();
            subscriber->open(serverHost, Socket::OpenMode::CONNECT, true, chrono::milliseconds(0),
                             sourceAddress.c_str());
            const Frame subscriberFrame {.pairId = static_cast<uint32_t>(pairId), .role = roleSubscriber};
            subscriber->write(bit_cast<const uint8_t*>(&subscriberFrame), frameSize);

            auto publisher = make_shared<TCPSocket>();
            publisher->open(serverHost, Socket::OpenMode::CONNECT, true, chrono::milliseconds(0),
                            sourceAddress.c_str());
            publisher->setOption(IPPROTO_TCP, TCP_NODELAY, 1);
            const Frame publisherFrame {.pairId = static_cast<uint32_t>(pairId), .role = rolePublisher};
            publisher->write(bit_cast<const uint8_t*>(&publisherFrame), frameSize);

            subscribers[pairId] = subscriber;
            publishers[pairId] = publisher;
        }
    };

    {
        vector<thread> connectors;
        const auto     perThread = (pairCount + connectThreadCount - 1) / connectThreadCount;
        for (size_t i = 0; i < connectThreadCount; ++i)
        {
            const auto from = min(i * perThread, pairCount);
            const auto to = min(from + perThread, pairCount);
            connectors.emplace_back([&connectRange, from, to] { connectRange(from, to); });
        }
        for (auto& connector: connectors)
        {
            connector.join();
        }
    }

    // Arm the subscribers only once every pair is registered, so no frame is routed to a
    // subscriber that is not being read yet.
    for (size_t pairId = 0; pairId < pairCount; ++pairId)
    {
        auto& subscriber = subscribers[pairId];
        subscriber->setOption(IPPROTO_TCP, TCP_NODELAY, 1);
        subscriber->blockingMode(false);
        pools[pairId % pools.size()].events->add(subscriber, subscriber);
    }

    // Publish one frame per publisher per second, spread across the second rather than bursted,
    // matching independently timed MQTT publishers.
    atomic<bool>   publishing {true};
    atomic<size_t> published {0};
    vector<thread> publisherThreads;
    for (size_t threadIndex = 0; threadIndex < publishThreadCount; ++threadIndex)
    {
        publisherThreads.emplace_back(
            [&, threadIndex]
            {
                // Spread each interval over slices so publishers arrive continuously rather than
                // as one burst per interval. At rates fast enough that the interval is itself
                // shorter than a slice, every publisher simply fires every interval.
                const auto slicesPerInterval = max<size_t>(1, static_cast<size_t>(publishInterval / publishSlice));
                const auto sliceDuration = publishInterval / slicesPerInterval;
                size_t     slice = 0;
                auto       nextWakeup = chrono::steady_clock::now();

                while (publishing.load(memory_order_relaxed))
                {
                    nextWakeup += sliceDuration;

                    // This thread owns every publishThreadCount'th publisher, and sends to a
                    // 1/slicesPerInterval share of them per slice - so each publisher is served
                    // exactly once per publishInterval. Stepping straight to the scheduled pairs
                    // keeps the harness's own per-slice cost proportional to what it sends.
                    const auto stride = publishThreadCount * slicesPerInterval;
                    for (size_t pairId = threadIndex + publishThreadCount * slice; pairId < pairCount;
                         pairId += stride)
                    {
                        const Frame frame {.pairId = static_cast<uint32_t>(pairId),
                                           .role = rolePublisher,
                                           .timestampNs = nowNs()};
                        try
                        {
                            publishers[pairId]->write(bit_cast<const uint8_t*>(&frame), frameSize);
                            published.fetch_add(1, memory_order_relaxed);
                        }
                        catch (const Exception&)
                        {
                            // Server is not draining this publisher; skip the tick.
                        }
                    }

                    slice = (slice + 1) % slicesPerInterval;

                    // If a slice overran, resynchronize instead of trying to catch up: the backlog
                    // would come out as a burst, which is not what independently timed publishers do.
                    const auto now = chrono::steady_clock::now();
                    if (nextWakeup < now)
                    {
                        nextWakeup = now;
                    }
                    this_thread::sleep_until(nextWakeup);
                }
            });
    }

    this_thread::sleep_for(warmupDuration);
    collecting.store(true, memory_order_relaxed);

    // Sample the receive queues while measuring: a queue that keeps growing is the signature of a
    // hand-off that cannot keep up, and separates that from latency the reactor itself caused.
    size_t     peakQueue = 0;
    const auto cpuTicksAtStart = server.serverCpuTicks();
    const auto routedAtStart = server.routed();
    const auto measureUntil = chrono::steady_clock::now() + measureDuration;
    while (chrono::steady_clock::now() < measureUntil)
    {
        peakQueue = max(peakQueue, server.maxQueueLength());
        this_thread::sleep_for(chrono::milliseconds(50));
    }
    const auto cpuTicksUsed = server.serverCpuTicks() - cpuTicksAtStart;
    const auto routedWhileMeasuring = server.routed() - routedAtStart;
    collecting.store(false, memory_order_relaxed);

    publishing.store(false, memory_order_relaxed);
    for (auto& publisherThread: publisherThreads)
    {
        publisherThread.join();
    }

    vector<uint64_t> samplesNs;
    for (auto& pool: pools)
    {
        pool.events->stop();
        samplesNs.insert(samplesNs.end(), pool.samplesNs.begin(), pool.samplesNs.end());
    }

    auto stats = summarize(std::move(samplesNs));
    stats.routed = server.routed();
    stats.dropped = server.dropped();
    stats.published = published.load();
    stats.peakQueue = peakQueue;

    COUT("  trigger=" << triggerModeName(triggerMode) << " maxEvents=" << maxEvents
                      << " rate=" << publishesPerSecond << "/s"
                      << " receiveThreads=" << (receiveThreadCount == 0 ? String("inline") : String(to_string(receiveThreadCount)))
                      << " queue=" << (receiveThreadCount == 0 ? "n/a" : (sharedQueue ? "shared" : "pinned"))
                      << " workUs=" << workMicroseconds << " peakQueue=" << peakQueue << endl);
    COUT("  pairs=" << pairCount << " connections=" << pairCount * 2 << " registered=" << server.registrations()
                    << " published=" << published.load() << " routed=" << stats.routed << " dropped=" << stats.dropped
                    << endl);
    COUT("  round trip: median=" << stats.medianUs << "us mean=" << stats.meanUs << "us p99=" << stats.p99Us
                                 << "us max=" << stats.maxUs << "us over " << stats.samples << " samples" << endl);

    const auto ticksPerSecond = static_cast<double>(sysconf(_SC_CLK_TCK));
    if (ticksPerSecond > 0 && cpuTicksUsed > 0)
    {
        const auto cpuSeconds = static_cast<double>(cpuTicksUsed) / ticksPerSecond;
        const auto cores = cpuSeconds / static_cast<double>(measureDuration.count());
        COUT("  server CPU: " << cores << " cores over the window ("
                              << (routedWhileMeasuring > 0
                                      ? cpuSeconds * 1e6 / static_cast<double>(routedWhileMeasuring)
                                      : 0.0)
                              << "us per routed message, excludes this test's client)" << endl);
    }

    for (auto& publisher: publishers)
    {
        publisher->close();
    }
    for (auto& subscriber: subscribers)
    {
        subscriber->close();
    }
    server.stop();

    return stats;
}

} // namespace

namespace sptk {

/**
 * @brief Baseline: round trip latency must not collapse as the connection count grows.
 *
 * Five times the connections and five times the message rate currently cost at most ~3.4x on the
 * median, in the worst trigger mode. The 10x bound leaves room for run-to-run noise while still
 * catching any change that puts real per-connection work back on the dispatch path.
 */
TEST(FastTcpServerScaleTests, DISABLED_roundTripScaling)
{
    const auto smallPairs = envSize("SPTK_SCALE_PAIRS_SMALL", smallPairCount);
    const auto largePairs = envSize("SPTK_SCALE_PAIRS_LARGE", largePairCount);
    const auto config = configFromEnv();

    COUT("Small scenario:" << endl);
    const auto small = runScenario(smallPairs, smallScenarioPort, config);

    // Let the small scenario's sockets leave TIME_WAIT before the large one claims ports.
    this_thread::sleep_for(chrono::seconds(10));

    COUT("Large scenario:" << endl);
    const auto large = runScenario(largePairs, largeScenarioPort, config);

    ASSERT_GT(small.samples, 0U) << "Small scenario collected no round trip samples";
    ASSERT_GT(large.samples, 0U) << "Large scenario collected no round trip samples";

    const auto ratio = large.medianUs / small.medianUs;
    COUT(endl
         << "Round trip median: " << small.medianUs << "us at " << smallPairs << " pairs vs " << large.medianUs
         << "us at " << largePairs << " pairs - " << ratio << "x" << endl);

    if (config.receiveThreadCount != 0 || config.workMicroseconds != 0)
    {
        // Re-pointed at some other load through the environment: report, don't judge. The bound
        // below only describes the reactor running the routing itself.
        return;
    }

    EXPECT_EQ(0U, small.dropped) << "Small scenario dropped frames: the server could not keep up";
    EXPECT_EQ(0U, large.dropped) << "Large scenario dropped frames: the server could not keep up";

    EXPECT_LE(ratio, 10.0) << "Round trip median degraded " << ratio << "x going from " << smallPairs << " to "
                           << largePairs << " pairs; the reactor sustained at most ~3.4x when this baseline "
                           << "was taken, so something now scales with the connection count";
}

/**
 * @brief Reproduces XMQ's latency cliff, and localizes it in the receive thread hand-off.
 *
 * Identical to the scenario above except that routing is handed to a fixed number of pinned
 * receive threads - XMQ's ClientSessionReceiveThread arrangement, at its configured
 * receive_threads: 3 - and each message costs some CPU, standing in for MQTT parsing, topic
 * lookup and QoS 1 bookkeeping.
 *
 * That pool is a fixed amount of service capacity, so it has a knee. With W microseconds per
 * message and R threads it saturates once the message rate passes R/W per second: at 100us and
 * 3 threads that is ~30K messages/second. 10K pairs (10K msg/s) sit at ~33% utilization and stay
 * fast; 50K pairs (50K msg/s) need ~167% and the queue grows without bound, which is what turns a
 * 5x load increase into four orders of magnitude of latency. Measured 184us vs 3.29s - 17918x,
 * against XMQ's reported ~300us and ~3s.
 *
 * The reactor is not involved: DISABLED_roundTripScaling runs the same connections and the same
 * message rate through it and stays flat.
 *
 * Defaults to the pinned arrangement at XMQ's 3 threads because that is the configuration being
 * reproduced. Raising SPTK_SCALE_RECEIVE_THREADS to 8 lifts capacity above the load and the cliff
 * disappears; see the pool size vs queue type table at the top of this file.
 */
TEST(FastTcpServerScaleTests, DISABLED_receiveThreadSaturation)
{
    const auto smallPairs = envSize("SPTK_SCALE_PAIRS_SMALL", smallPairCount);
    const auto largePairs = envSize("SPTK_SCALE_PAIRS_LARGE", largePairCount);

    auto config = configFromEnv();
    config.receiveThreadCount = envSize("SPTK_SCALE_RECEIVE_THREADS", 3);
    config.workMicroseconds = envSize("SPTK_SCALE_WORK_US", 100);

    COUT("Small scenario:" << endl);
    const auto small = runScenario(smallPairs, smallScenarioPort, config);

    this_thread::sleep_for(chrono::seconds(10));

    COUT("Large scenario:" << endl);
    const auto large = runScenario(largePairs, largeScenarioPort, config);

    ASSERT_GT(small.samples, 0U) << "Small scenario collected no round trip samples";
    ASSERT_GT(large.samples, 0U) << "Large scenario collected no round trip samples";

    const auto ratio = large.medianUs / small.medianUs;
    COUT(endl
         << "Round trip median: " << small.medianUs << "us at " << smallPairs << " pairs vs " << large.medianUs
         << "us at " << largePairs << " pairs - " << ratio << "x" << endl);

    // The small scenario must stay below the knee, otherwise the comparison says nothing.
    EXPECT_LT(small.medianUs, 5000.0) << "Small scenario was already saturated at " << small.medianUs
                                      << "us; lower SPTK_SCALE_WORK_US or raise SPTK_SCALE_RECEIVE_THREADS";
    EXPECT_LT(small.peakQueue, 1000U) << "Small scenario's receive queue was already backing up";

    // The large one must be over it: an unbounded queue, and the message rate no longer sustained.
    EXPECT_GT(large.peakQueue, 10000U) << "Large scenario's receive queue did not back up, so the "
                                       << "hand-off was not the limiting factor in this run";
    EXPECT_LT(large.routed, large.published) << "Large scenario kept up, so it never reached saturation";

    EXPECT_GE(ratio, 100.0) << "Expected the receive thread pool to saturate between " << smallPairs << " and "
                            << largePairs << " pairs and cost >=100x on the median, got " << ratio << "x";
}

} // namespace sptk
