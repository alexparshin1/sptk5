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
#include <sptk5/threads/SynchronizedQueue.h>

namespace smq {

class SP_EXPORT Message : public sptk::Buffer
{
public:
    typedef std::map<sptk::String, sptk::String> Headers;

    enum Type : uint8_t {
        UNDEFINED       = 0,
        CONNECT         = 1,
        DISCONNECT      = 2,
        SUBSCRIBE       = 3,
        UNSUBSCRIBE     = 4,
        PING            = 5,
        MESSAGE         = 6,
        CONNECT_ACK     = 7,
        SUBSCRIBE_ACK   = 8,
        PUBLISH_ACK     = 9,
        UNSUBSCRIBE_ACK = 10,
        PING_ACK        = 11
    };


private:
    Headers         m_headers;
    sptk::DateTime  m_created;
    Type            m_type {MESSAGE};
    sptk::String    m_destination;

public:

    explicit Message(Type type=MESSAGE)
    : m_created("now"), m_type(type) {}

    Message(const Message& other) = default;

    Message(Type type, const Buffer& other)
    : Buffer(other),
      m_created("now"),
      m_type(type)
    {
    }

    Message(Type type, const Buffer&& other)
    : Buffer(other),
      m_created("now"),
      m_type(type)
    {
    }

    Message(Message&& other) noexcept
    : Buffer(std::move(other)),
      m_headers(std::move(other.m_headers)),
      m_created(other.m_created),
      m_type(other.m_type),
      m_destination(std::move(other.m_destination))
    {}

    ~Message() override = default;

    void clear();

    Message& operator=(const Message& other) = default;
    Message& operator=(Message&& other) noexcept;

    sptk::String& operator[](const sptk::String& header);
    sptk::String  operator[](const sptk::String& header) const;

    const sptk::DateTime created() const  { return m_created; }

    Headers& headers() { return m_headers; }
    const Headers& headers() const { return m_headers; }

    sptk::String toString() const;

    Type type() const { return m_type; }

    static sptk::String typeToString(Type type);

    sptk::String destination() const;

    void destination(const sptk::String& destination);
};

typedef std::shared_ptr<Message>    SMessage;
typedef sptk::SynchronizedQueue<SMessage> SMessageQueue;

}

#endif
