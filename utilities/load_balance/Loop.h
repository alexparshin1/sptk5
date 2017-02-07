#ifndef __LOOP__
#define __LOOP__

#include <vector>
#include <sptk5/Exception.h>

template <class T> class Loop
{
    std::vector<T> m_list;
    size_t          m_position;
public:
    Loop()
    : m_position(0)
    {
    }

    void add(const T& data)
    {
        m_list.push_back(data);
    }

    const T& loop()
    {
        if (m_list.empty())
            throw new sptk::Exception("Loop is empty");

        size_t currentPosition = m_position;
        m_position++;
        if (m_position >= m_list.size())
            m_position = 0;
        const T& item = m_list[currentPosition];
        return item;
    }
};

#endif
