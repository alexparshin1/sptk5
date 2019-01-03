#ifndef __SMQ_MESSAGE_H__
#define __SMQ_MESSAGE_H__

#include <src/sputil/mq/Message.h>
#include <sptk5/net/TCPSocket.h>

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
};

} // namespace sptk

#endif