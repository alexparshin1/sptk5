#include <sptk5/mq/SMQMessage.h>

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

shared_ptr<Message> SMQMessage::readConnect(TCPSocket& socket)
{
    uint32_t    messageSize;
    String      clientId;
    String      username;
    String      password;

    SMQMessage::read(socket, messageSize);
    SMQMessage::read(socket, clientId);
    SMQMessage::read(socket, username);
    SMQMessage::read(socket, password);

    auto message = make_shared<Message>(Message::CONNECT);
    (*message)["clientid"] = clientId;
    (*message)["username"] = username;
    (*message)["password"] = password;

    return message;
}

shared_ptr<Message> SMQMessage::readRawMessage(TCPSocket& socket)
{
    char    data[16];
    String  destination;
    Buffer  message;
    uint8_t messageType;

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
            return SMQMessage::readConnect(socket);

        case Message::MESSAGE:
        case Message::SUBSCRIBE:
            SMQMessage::read(socket, destination);
            SMQMessage::read(socket, message);
            break;

        default:
            destination = "";
            message.bytes(0);
            break;
    }

    auto msg = make_shared<Message>((Message::Type) messageType, move(message));
    msg->destination(destination);

    return msg;
}
