#include "SMQMessage.h"

using namespace std;
using namespace sptk;

void SMQMessage::read(TCPSocket& socket, String& str)
{
    uint8_t dataSize;
    read(socket, dataSize);
    if (dataSize == 0)
        throw Exception("Invalid string size");
    socket.read(str, dataSize);
}

void SMQMessage::read(TCPSocket& socket, Buffer& data)
{
    uint32_t dataSize;
    read(socket, dataSize);
    if (dataSize > 0) {
        data.checkSize(dataSize);
        socket.read(data.data(), dataSize);
    }
    data.bytes(dataSize);
}

