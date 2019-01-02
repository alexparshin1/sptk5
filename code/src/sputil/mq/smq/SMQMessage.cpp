#include "SMQMessage.h"

using namespace std;
using namespace sptk;

void SMQMessage::send(TCPSocket& socket)
{
    Buffer output;

    if (!socket.active())
        throw Exception("Not connected");

    // Append message type
    output.append((uint8_t)type());

    if (type() == MESSAGE || type() == SUBSCRIBE) {
        if (destination().empty())
            throw Exception("Empty destination");
        // Append destination
        output.append((uint8_t) destination().size());
        output.append(destination());
    }

    output.append((uint32_t)bytes());
    output.append(c_str(), bytes());

    const char* magic = "MSG:";
    socket.write(magic, strlen(magic));
    socket.write(output);
}

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

void SMQMessage::readConnect(TCPSocket& socket)
{
    uint32_t    messageSize;
    String      username;
    String      password;
    read(socket, messageSize);
    read(socket, username);
    read(socket, password);
    (*this)["username"] = username;
    (*this)["password"] = password;
}

void SMQMessage::readMessage(TCPSocket& socket)
{
    String _destination;
    read(socket, _destination);
    destination(_destination);

    read(socket, *(Buffer*)this);
}

void SMQMessage::receive(TCPSocket& socket, Buffer& message, uint8_t& messageType)
{
    char data[16];

    // Read message signature
    socket.read(data, 4);
    if (strncmp(data, "MSG:", 4) != 0)
        throw Exception("Invalid message magic byte");

    // Read message type
    socket.read((char*)&messageType, sizeof(messageType));
    if (messageType > Message::MESSAGE)
        throw Exception("Invalid message type");

    switch (messageType) {
        case Message::CONNECT:
            readConnect(socket);
            break;
        case Message::MESSAGE:
        case Message::SUBSCRIBE:
        case Message::UNSUBSCRIBE:
            readMessage(socket);
            break;
        default:
            clear();
            break;
    }
}
