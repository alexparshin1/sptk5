/*
┌──────────────────────────────────────────────────────────────────────────────┐
│                            Edulog Data Portal                                │
│                              version 1.1.17                                  │
│                                                                              │
│                      Copyright © 2015-2018 Education Logistics.              │
│                            All Rights Reserved                               │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#ifndef __MESSAGE_H__
#define __MESSAGE_H__

#include <sptk5/Buffer.h>
#include <sptk5/Strings.h>
#include <sptk5/DateTime.h>

namespace sptk {

class Message : public Buffer
{
public:
    typedef std::map<String, String> Headers;

    enum Type {
        UNDEFINED    = 0,
        CONNECT      = 1,
        DISCONNECT   = 2,
        SUBSCRIBE    = 4,
        UNSUBSCRIBE  = 8,
        PING         = 16,
        MESSAGE      = 32
    };


private:
    Headers         m_headers;
    DateTime        m_created;
    Type            m_type { MESSAGE };

public:

    explicit Message(Type type=MESSAGE)
    : m_created("now"), m_type(type) {}

    Message(const Message& other) = default;

    Message(Type type, const Buffer& other, const String& destination = "")
    : Buffer(other),
      m_created("now"),
      m_type(type)
    {
        m_headers["destination"] = destination;
    }

    Message(Type type, const Buffer&& other, const String& destination = "")
    : Buffer(other),
      m_created("now"),
      m_type(type)
    {
        m_headers["destination"] = destination;
    }

    Message(Message&& other) noexcept
    : Buffer(std::move(other)),
      m_headers(std::move(other.m_headers)),
      m_created(other.m_created),
      m_type(other.m_type)
    {}

    virtual ~Message() = default;

    void clear();

    Message& operator=(const Message& other) = default;
    Message& operator=(Message&& other) noexcept;

    String& operator[](const String& header);
    String  operator[](const String& header) const;

    const DateTime created() const  { return m_created; }

    Headers& headers() { return m_headers; }
    const Headers& headers() const { return m_headers; }

    String toString() const;

    Type type() const { return m_type; }

    static String typeToString(Type type);

    const String& destination() const;

    void destination(const String& destination);
};

}

#endif
