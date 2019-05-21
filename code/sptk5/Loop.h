#ifndef __LOOP__
#define __LOOP__

#include <sptk5/Exception.h>
#include <list>
#include <mutex>

template <class T> class Loop
{
    std::list<T>                    m_list;
    typename std::list<T>::iterator m_position;
public:

    Loop()
    {
        m_position = m_list.end();
    }

    void clear()
    {
        m_position = m_list.end();
        m_list.clear();
    }

    void add(const T& data)
    {
        m_list.push_back(data);
        if (m_list.size() == 1)
            m_position = m_list.begin();
    }

    T& get()
    {
        if (m_list.empty())
            throw sptk::Exception("Loop is empty");
        return *m_position;
    }

    T& loop()
    {
        if (m_list.empty())
            throw sptk::Exception("Loop is empty");
        ++m_position;
        if (m_position == m_list.end())
            m_position = m_list.begin();
        return *m_position;
    }

    size_t size() const
    {
        return m_list.size();
    }
};

#endif
