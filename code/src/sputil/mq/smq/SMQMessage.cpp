#include <sptk5/mq/SMQMessage.h>
#include <sptk5/cutils>

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

static void parseHeaders(Buffer& buffer, Message::Headers& headers)
{
    char* pstart = buffer.data();
    char* pend = buffer.data() + buffer.bytes();
    for (char* ptr = pstart; ptr < pend; ) {
        char* nameEnd = strstr(ptr, ": ");
        if (nameEnd == nullptr)
            break;
        *nameEnd = 0;
        char* valueStart = nameEnd + 2;
        char* valueEnd = strchr(valueStart, '\n');
        if (valueStart != nullptr)
            *valueEnd = 0;
        headers[ptr] = valueStart;
        ptr = valueEnd + 1;
    }
}

shared_ptr<Message> SMQMessage::readRawMessage(TCPSocket& socket)
{
    char    data[16];
    Buffer  headers;
    Buffer  message;
    uint8_t messageType;

    // Read message signature
    size_t bytes = socket.read(data, 4);
    if (bytes == 0)
        throw ConnectionException("Connection closed");
    if (strncmp(data, "MSG:", 4) != 0)
        throw Exception("Invalid message magic byte");

    // Read message type
    socket.read((char*)&messageType, sizeof(messageType));
    if (messageType > Message::MESSAGE)
        throw Exception("Invalid message type");

    switch (messageType) {
        case Message::CONNECT:
        case Message::SUBSCRIBE:
            SMQMessage::read(socket, headers);
            break;

        case Message::MESSAGE:
            SMQMessage::read(socket, headers);
            SMQMessage::read(socket, message);
            break;

        default:
            message.bytes(0);
            break;
    }

    auto msg = make_shared<Message>((Message::Type) messageType, move(message));
    if (!headers.empty())
        parseHeaders(headers, msg->headers());

    return msg;
}

void SMQMessage::sendMessage(TCPSocket& socket, const Message& message)
{
    Buffer output("MSG:", 4);

    if (!socket.active())
        throw Exception("Not connected");

    // Append message type
    output.append((uint8_t)message.type());

    Buffer headers;
    for (auto itor: message.headers()) {
        headers.append(itor.first);
        headers.append(": ", 2);
        headers.append(itor.second);
        headers.append('\n');
    }
    output.append((uint32_t) headers.bytes());
    output.append(headers.c_str(), headers.bytes());

    if ((message.type() & (Message::MESSAGE|Message::SUBSCRIBE)) != 0) {
        if (message.destination().empty()) {
            throw Exception("Message destination is empty or not defined");
        }

        if (message.type() == Message::MESSAGE) {
            output.append((uint32_t) message.bytes());
            output.append(message.c_str(), message.bytes());
        }
    }

    socket.write(output);
}
