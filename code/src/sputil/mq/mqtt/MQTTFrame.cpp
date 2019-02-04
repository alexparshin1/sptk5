/*
┌──────────────────────────────────────────────────────────────────────────────┐
│                               vSPARC Server                                  │
│                               version 1.0.0                                  │
│                                                                              │
│                             Messaging library                                │
│                                                                              │
│                        Copyright Telstra Corporation Ltd                     │
│                            All Rights Reserved                               │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#include "mqtt/MQTTFrame.h"

using namespace std;
using namespace vsparc;
using namespace vsparc::mqtt;

uint16_t Frame::nextPacketId()
{
    static mutex nextPacketIdMutex;
    static auto packetId = (uint16_t) time(nullptr);

    lock_guard<mutex>   guard(nextPacketIdMutex);

    return ++packetId;
}

void Frame::appendShortValue(uint16_t value)
{
    append(htons(value));
}

void Frame::appendVariableHeader(const char* data, uint16_t dataLength)
{
    appendShortValue(dataLength);
    if (dataLength != 0)
        append(data, dataLength);
}

void Frame::appendVariableHeader(const string& data)
{
    appendShortValue((uint16_t)data.length());
    if (!data.empty())
        append(data);
}

void Frame::appendRemainingLength(unsigned remainingLength)
{
    unsigned X = remainingLength;
    do {
        auto digit = uint8_t(X % 128);
        X /= 128;
        // if there are more digits to encode, set the top bit of this digit
        if ( X > 0 )
            digit |= 0x80;
        append(digit);
    } while ( X> 0 );
}

Frame::Frame(MessageType type, uint16_t id, QOS qos)
: m_type(type), m_id(id), m_qos(qos)
{}

const Buffer& Frame::connectFrame(uint16_t keepAliveSeconds, string username, string password, string clientId, std::string willTopic,
                                  MQTTProtocol protocol, const Host& host)
{
    bytes(0);

    string  protocolName;
    int     headerlen = 12;
    uint8_t protocolVersion = 3;

    switch (protocol) {
        case MQTT_PROTOCOL_V31:
            headerlen = 12;
            protocolName = "MQIsdp";
            protocolVersion = 3;
            break;
        case MQTT_PROTOCOL_V311:
            headerlen = 10;
            protocolName = "MQTT";
            protocolVersion = 4;
            break;
    }

    uint8_t connectFlags = 0;
    int payloadlen = (int) clientId.length() + 2;

    // If username and password are defined - add their size to payload length
    if (!username.empty()) {
        connectFlags |= CF_USERNAME;
        payloadlen += username.length() + 2;
        if (!password.empty()) {
            connectFlags |= CF_PASSWORD;
            payloadlen += password.length() + 2;
        }
    }

    string willMessage;
    // If will topic and message are defined - add their size to payload length
    if (!willTopic.empty()) {
        MQMessage will;
        will["client_id"] = clientId;
        will["broker_host"] = host.toString();
        will["subject"] = "connection_terminated";
        will.body().append("Connection to MQTT broker terminated");
        willMessage = will.toString();

        connectFlags |= CF_WILL_QOS_1 | CF_WILL_FLAG;
        payloadlen += willTopic.length() + 2;
        payloadlen += willMessage.length() + 2;
    }

    append((uint8_t) MT_CONNECT);
    appendRemainingLength((unsigned int) (headerlen + payloadlen));        // Remaining Length
    appendVariableHeader(protocolName);                 // Protocol Name
    append((uint8_t) protocolVersion);                  // Protocol Version

    append(connectFlags);                               // Connect Flags
    appendShortValue(keepAliveSeconds);

    // Payload

    if (clientId.empty())
        throw Exception("Client Id shouldn't be empty");
    appendVariableHeader(clientId);                 // Client id

    if (!willTopic.empty()) {
        appendVariableHeader(willTopic);
        appendVariableHeader(willMessage);
    }

    if (!username.empty()) {
        appendVariableHeader(username);
        if (!password.empty())
            appendVariableHeader(password);
    }

    return *this;
}

const Buffer& Frame::publishFrame(const string& topic, const Buffer& data, QOS qos, bool dup, bool retain)
{
    bytes(0);

    auto packetlen = unsigned(topic.length() + 2 + data.bytes());
    if (qos != QOS_0)
        packetlen += 2; // Two bytes of message id

    // Fixed header
    int dupFlag = dup ? 1 : 0;
    int retainFlag = retain ? 1 : 0;
    append((uint8_t) (unsigned) (MT_PUBLISH|(dupFlag<<3)|(qos << 1)|retainFlag));
    appendRemainingLength(packetlen);

    // Variable header
    appendVariableHeader(topic);
    if (qos != QOS_0) {
        m_id = nextPacketId();
        appendShortValue(m_id);
    } else {
        m_id = 0;
    }

    // Payload
    append(data);

    return *this;
}

const Buffer& Frame::publishAckFrame(uint16_t messageId, MessageType ackType)
{
    bytes(0);

    int packetlen = 2;

    // Fixed header
    append((uint8_t) ackType);
    append((uint8_t) packetlen);                            // Remaining Length

    appendShortValue(messageId);

    return *this;
}

const Buffer& Frame::subscribeFrame(const string& topic, QOS qos)
{
    bytes(0);

    auto packetlen = unsigned((topic.length() + 2) + 2 + 1);

    // Fixed header
    append((uint8_t) (unsigned) (MT_SUBSCRIBE|(1 << 1)));
    appendRemainingLength(packetlen);

    // Variable header
    if (qos != QOS_0) {
        m_id = nextPacketId();
        appendShortValue(m_id);
    } else {
        m_id = 0;
    }

    // Payload
    appendVariableHeader(topic);
    append((uint8_t) qos);

    return *this;
}

const Buffer& Frame::subscribeFrame(const Strings& topics, QOS qos)
{
    bytes(0);

    unsigned packetlen = 2;
    for (auto& topic: topics)
        packetlen += (topic.length() + 2) + 1;

    // Fixed header
    append((uint8_t) (unsigned) (MT_SUBSCRIBE|(1 << 1)));
    appendRemainingLength(packetlen);

    // Variable header
    if (qos != QOS_0) {
        m_id = nextPacketId();
        appendShortValue(m_id);
    } else {
        m_id = 0;
    }

    // Payload
    for (auto& topic: topics) {
        appendVariableHeader(topic);
        append((uint8_t) qos);
    }

    return *this;
}

const Buffer& Frame::unsubscribeFrame(const string& topic, QOS qos)
{
    bytes(0);

    unsigned packetlen = unsigned(topic.length() + 2) + 2;

    // Fixed header
    append((uint8_t) (unsigned) (MT_UNSUBSCRIBE|(1 << 1)));
    appendRemainingLength(packetlen);

    // Variable header
    if (qos != QOS_0) {
        m_id = nextPacketId();
        appendShortValue(m_id);
    } else {
        m_id = 0;
    }

    // Payload
    appendVariableHeader(topic);

    return *this;
}

const Buffer& Frame::unsubscribeFrame(const Strings& topics, QOS qos)
{
    bytes(0);

    unsigned packetlen = 2;
    for (auto& topic: topics)
        packetlen += (topic.length() + 2);

    // Fixed header
    append((uint8_t) (unsigned) (MT_UNSUBSCRIBE|(1 << 1)));
    appendRemainingLength(packetlen);

    // Variable header
    if (qos != QOS_0) {
        m_id = nextPacketId();
        appendShortValue(m_id);
    } else {
        m_id = 0;
    }

    // Payload
    for (auto& topic: topics)
        appendVariableHeader(topic);

    return *this;
}

const Buffer& Frame::pingFrame()
{
    bytes(0);

    // Fixed header
    append((uint8_t) (unsigned) MT_PINGREQ);
    appendRemainingLength(0);

    return *this;
}

unsigned Frame::receiveRemainingLength(Socket& connection)
{
    uint8_t     digit;
    int         multiplier = 1;
    unsigned    value = 0;

    do {
        connection.read((char*)&digit, 1);
        value += (digit & 127) * multiplier;
        multiplier *= 128;
    } while ((digit & 128) != 0);
    return value;
}

MessageType Frame::ackType(MessageType messageType)
{
    MessageType _ackType;
    switch (messageType) {
        case MT_CONNECT:        _ackType = MT_CONNACK; break;
        case MT_PUBLISH:        _ackType = MT_PUBACK; break;
        case MT_SUBSCRIBE:      _ackType = MT_SUBACK; break;
        case MT_UNSUBSCRIBE:    _ackType = MT_UNSUBACK; break;
        default:                _ackType = MT_UNDEFINED; break;
    }
    return _ackType;
}

uint16_t Frame::receiveShortValue(Socket& connection)
{
    uint16_t len;
    connection.read((char*)&len, 2);
    return ntohs(len);
}

void Frame::receiveVariableHeader(Socket& connection, string& header)
{
    uint16_t len = receiveShortValue(connection);
    connection.read(header, len);
}

void Frame::receiveConnectACK(Socket& connection)
{
    uint8_t connectACK(0), connectRC;

    connection.read((char*)&connectACK, 1);
    connection.read((char*)&connectRC, 1);

    switch (connectRC) {
        case 0: break;
        case 1: throw Exception("Unacceptable protocol version");
        case 2: throw Exception("Identifier rejected");
        case 3: throw Exception("Server unavailable");
        case 4: throw Exception("Bad user name or password");
        case 5: throw Exception("Not authorized");
        default: throw Exception("Reason unknown");
    }
}

void Frame::receivePublishFrame(Socket& connection, unsigned remainingLength, QOS qos, string& topicName)
{
    int variableHeaderLength = 0;

    receiveVariableHeader(connection, topicName);
    variableHeaderLength += topicName.length() + 2;

    if (qos != QOS_0) {
        uint16_t id;
        connection.read((char*)&id, 2);
        m_id = ntohs(id);
        variableHeaderLength += 2;
    }

    unsigned dataLength = remainingLength - variableHeaderLength;

    checkSize(dataLength + 1);
    bytes(dataLength);

    connection.read(*this, dataLength);
    m_buffer[dataLength] = 0;
}

void Frame::receiveAnyFrame(Socket& socket, unsigned remainingLength)
{
    checkSize(remainingLength + 1);
    socket.read(*this, remainingLength);
    bytes(remainingLength);
    m_buffer[remainingLength] = 0;
}

int Frame::receive(Socket& connection, uint32_t timeoutMS, string& topicName)
{
    if (timeoutMS > 0 && !connection.waitForRead(timeoutMS))
        return 0;

    auto availableBytes = (unsigned) connection.getAvailableBytes();
    if (availableBytes == 0)
        throw Exception("Server closed connection");

    // Read fixed header
    uint8_t   messageHeader;
    connection.read((char*)&messageHeader, 1);
    m_type = (MessageType) (messageHeader & 0xF0);
    m_qos = (QOS) ((messageHeader & 0x06) >> 1);
    unsigned remainingLength = receiveRemainingLength(connection);

    switch (m_type) {
        case MT_UNDEFINED:
            return 0;

        // CONNECT ACK
        case MT_CONNACK:
            receiveConnectACK(connection);
            break;

        // PUBLISH
        case MT_PUBLISH:
            receivePublishFrame(connection, remainingLength, m_qos, topicName);
            break;

        // PUB ACK
        case MT_PUBACK:
            if (remainingLength != 2)
                THROW_EXCEPTION("Invalid remaining length for PUBACK: " << remainingLength);
            receiveAnyFrame(connection, remainingLength);
            break;

        // Other ACKs
        case MT_SUBACK:
        case MT_UNSUBACK:
            receiveAnyFrame(connection, remainingLength);
            break;

        default:
            receiveAnyFrame(connection, remainingLength);
            break;
    }

    return 1;
}

std::string Frame::typeName() const
{
    switch (m_type) {
        case MT_UNDEFINED:  return "UNDEFINED";
        case MT_CONNECT:    return "CONNECT";
        case MT_CONNACK:    return "CONN ACK";
        case MT_PUBLISH:    return "PUBLISH";
        case MT_PUBACK:     return "PUB ACK";
        case MT_PUBREC:     return "PUB REC";
        case MT_PUBREL:     return "PUB REL";
        case MT_PUBCOMP:    return "PUB COMP";
        case MT_SUBSCRIBE:  return "SUBSCRIBE";
        case MT_SUBACK:     return "SUB ACK";
        case MT_UNSUBSCRIBE: return "UNSUBSCRIBE";
        case MT_UNSUBACK:   return "UNSUB ACK";
        case MT_PINGREQ:    return "PING REQ";
        case MT_PINGRESP:   return "PING RESP";
        case MT_DISCONNECT: return "DISCONNECT";
    }
    return "UNDEFINED";
}
