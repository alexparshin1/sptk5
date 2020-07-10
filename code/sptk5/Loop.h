/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2020 by Alexey Parshin. All rights reserved.    ║
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

#ifndef __LOOP__
#define __LOOP__

#include <sptk5/Exception.h>
#include <list>
#include <mutex>

template <class T> class Loop
{
    mutable std::mutex              m_mutex;
    std::list<T>                    m_list;
    typename std::list<T>::iterator m_position;
public:

    Loop()
    {
        m_position = m_list.end();
    }

    void clear()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_position = m_list.end();
        m_list.clear();
    }

    void add(const T& data)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_list.push_back(data);
        m_position = m_list.end();
        m_position--;
    }

    T& get()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_list.empty())
            throw sptk::Exception("Loop is empty");
        return *m_position;
    }

    T& loop()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_list.empty())
            throw sptk::Exception("Loop is empty");
        ++m_position;
        if (m_position == m_list.end())
            m_position = m_list.begin();
        return *m_position;
    }

    size_t size() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_list.size();
    }
};

#endif
