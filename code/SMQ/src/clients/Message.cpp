/*
┌──────────────────────────────────────────────────────────────────────────────┐
│                            Edulog Data Portal                                │
│                              version 1.1.17                                  │
│                                                                              │
│                      Copyright © 2015-2018 Education Logistics.              │
│                            All Rights Reserved                               │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#include <smq/Message.h>

using namespace std;
using namespace sptk;
using namespace smq;

void Message::clear()
{
    m_headers.clear();
    m_type = UNDEFINED;
    bytes(0);
}

Message& Message::operator=(Message&& other) noexcept
{
    m_headers = std::move(other.m_headers);
    m_created = other.m_created;
    m_type = other.m_type;
    other.m_type = MESSAGE;
    other.m_created = DateTime();
    *(Buffer*)this = std::move(*(Buffer*)&other);
    return *this;
}

String& Message::operator[](const String& header)
{
    return m_headers[header];
}

String Message::operator[](const String& header) const
{
    auto itor = m_headers.find(header);
    if (itor == m_headers.end())
        return "";
    return itor->second;
}

String Message::toString() const
{
    stringstream output;
    output << typeToString(m_type) << endl;
    for (auto itor: m_headers) {
        output << itor.first << ": " << itor.second << endl;
    }
    output << "timestamp: " << m_created.isoDateTimeString(DateTime::PA_MILLISECONDS) << endl << endl;
    output << c_str() << endl;
    return output.str();
}

String Message::typeToString(Type type)
{
    switch (type) {
        case CONNECT:
            return "CONNECT";
        case DISCONNECT:
            return "DISCONNECT";
        case SUBSCRIBE:
            return "SUBSCRIBE";
        case UNSUBSCRIBE:
            return "UNSUBSCRIBE";
        case PING:
            return "PING";
        default:
            break;
    }
    return "MESSAGE";
}

String Message::destination() const
{
    return m_destination;
}

void Message::destination(const String& destination)
{
    m_destination = destination;
}

#if USE_GTEST
TEST(SPTK_Message,ctor)
{
    DateTime    started("now");
    Message     message;

    message = Message(Message::MESSAGE, Buffer("test data"));
	message["queue"] = "/test";

	EXPECT_STREQ("test data", message.c_str());
	EXPECT_STREQ("/test", message["queue"].c_str());
}

TEST(SPTK_Message, copy_ctor)
{
	DateTime    started("now");
	Message     message;

	message = Message(Message::MESSAGE, Buffer("test data"));
	message["queue"] = "/test";

	Message		message2(message);

	EXPECT_STREQ("test data", message2.c_str());
	EXPECT_STREQ("/test", message2["queue"].c_str());
}

TEST(SPTK_Message, move_ctor)
{
	DateTime    started("now");
	Message     message;

	message = Message(Message::MESSAGE, Buffer("test data"));
	message["queue"] = "/test";

	Message		message2(move(message));

	EXPECT_STREQ("", message.c_str());
	EXPECT_STREQ("", message["queue"].c_str());

	EXPECT_STREQ("test data", message2.c_str());
	EXPECT_STREQ("/test", message2["queue"].c_str());
}
#endif
