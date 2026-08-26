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

#pragma once

#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

namespace sptk {

/**
 * @brief A std::thread that joins when it is destroyed.
 *
 * It exists instead of std::jthread because on libc++ std::jthread needs -fexperimental-library,
 * which stamps an ABI tag naming the libc++ version onto every symbol that touches it, so a libc++
 * upgrade becomes an ABI break.
 */
class JoiningThread final
{
public:
    JoiningThread() = default;

    /**
     * @brief Start a thread, the same way std::thread is started.
     *
     * Constrained, or this would be a better match than the move constructor for a JoiningThread
     * argument and quietly take over moves - the same trap std::thread guards against.
     */
    template<typename Function, typename... Arguments>
        requires(!std::is_same_v<std::decay_t<Function>, JoiningThread>)
    explicit JoiningThread(Function&& function, Arguments&&... arguments)
        : m_thread(std::forward<Function>(function), std::forward<Arguments>(arguments)...)
    {
    }

    JoiningThread(const JoiningThread&) = delete;
    JoiningThread& operator=(const JoiningThread&) = delete;

    JoiningThread(JoiningThread&&) noexcept = default;

    JoiningThread& operator=(JoiningThread&& other) noexcept
    {
        // Join what is being replaced first: assigning over a running thread would otherwise
        // terminate, which is what std::thread's move assignment does.
        join();
        m_thread = std::move(other.m_thread);
        return *this;
    }

    ~JoiningThread()
    {
        join();
    }

    /**
     * @brief Join the thread if it is running. Doing it twice is harmless.
     */
    void join()
    {
        if (m_thread.joinable())
        {
            m_thread.join();
        }
    }

    [[nodiscard]] bool joinable() const
    {
        return m_thread.joinable();
    }

    [[nodiscard]] std::thread::id get_id() const noexcept
    {
        return m_thread.get_id();
    }

private:
    std::thread m_thread;
};

/**
 * @brief A group of threads, all joined when the group goes out of scope.
 */
using JoiningThreads = std::vector<JoiningThread>;

} // namespace sptk
