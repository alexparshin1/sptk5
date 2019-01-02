#ifndef __SMQ_MESSAGE_H__
#define __SMQ_MESSAGE_H__

#include <src/sputil/mq/Message.h>
#include <sptk5/net/TCPSocket.h>

namespace sptk {

class SMQMessage : public Message
{
    template<class T> void read(TCPSocket& socket, T& data)
    {
        socket.read((char*)&data, sizeof(data));
    }

    void read(TCPSocket& socket, String& str);
    void read(TCPSocket& socket, Buffer& data);

    void readConnect(TCPSocket& socket);
    void readMessage(TCPSocket& socket);
public:

    void receive(TCPSocket& socket, Buffer& message, uint8_t& messageType);
    void send(TCPSocket& socket);
};

} // namespace sptk

#endif