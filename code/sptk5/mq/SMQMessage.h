#ifndef __SMQ_MESSAGE_H__
#define __SMQ_MESSAGE_H__

#include <sptk5/mq/Message.h>
#include <sptk5/net/TCPSocket.h>
#include <sptk5/threads/SynchronizedQueue.h>

namespace sptk {

class SMQMessage : public Message
{
public:

    template<class T> static void read(TCPSocket& socket, T& data)
    {
        socket.read((char*)&data, sizeof(data));
    }

    static void read(TCPSocket& socket, String& str);
    static void read(TCPSocket& socket, Buffer& data);

    static std::shared_ptr<Message> readRawMessage(TCPSocket& socket);
    static void sendMessage(TCPSocket& socket, const Message& message);
};

typedef std::shared_ptr<Message>                    SMessage;
typedef SynchronizedQueue<SMessage>                 SMessageQueue;

} // namespace sptk

#endif