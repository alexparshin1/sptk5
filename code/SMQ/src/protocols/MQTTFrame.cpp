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

#include <smq/protocols/MQTTFrame.h>

using namespace std;
using namespace sptk;
using namespace chrono;

uint16_t MQTTFrame::nextPacketId()
{
    static mutex nextPacketIdMutex;
    static auto packetId = (uint16_t) time(nullptr);

    lock_guard<mutex>   guard(nextPacketIdMutex);

    return ++packetId;
}

void MQTTFrame::appendShortValue(uint16_t value)
{
    append(htons(value));
}

void MQTTFrame::appendVariableHeader(const char* data, uint16_t dataLength)
{
    appendShortValue(dataLength);
    if (dataLength != 0)
        append(data, dataLength);
}

void MQTTFrame::appendVariableHeader(const String& data)
{
    appendShortValue((uint16_t)data.length());
    if (!data.empty())
        append(data);
}

void MQTTFrame::appendRemainingLength(unsigned remainingLength)
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

MQTTFrame::MQTTFrame(MQTTFrameType type, uint16_t id, MQTTQOS qos)
: m_type(type), m_id(id), m_qos(qos)
{}

const Buffer& MQTTFrame::setCONNECT(uint16_t keepAliveSeconds, const String& username, const String& password,
                                    const String& clientId,
                                    const String& lastWillTopic, MQTTProtocolVersion protocolVersion)
{

    String  protocolName;
    int     headerlen;

    switch (protocolVersion) {
        case MQTT_PROTOCOL_V31:
            headerlen = 12;
            protocolName = "MQIsdp";
            break;
        case MQTT_PROTOCOL_V311:
            headerlen = 10;
            protocolName = "MQTT";
            break;
        default:
            throw Exception("Unsupported MQTT protocol version");
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

    String willMessage;
    // If will topic and message are defined - add their size to payload length
    if (!lastWillTopic.empty()) {
        Message will;
        will["event"] = "connection_terminated";
        will["client_id"] = clientId;
        will.append("Lost connection to MQTT broker");
        willMessage = will.toString();

        connectFlags |= CF_WILL_QOS_1 | CF_WILL_FLAG;
        payloadlen += lastWillTopic.length() + 2;
        payloadlen += willMessage.length() + 2;
    }

    bytes(0);
    append(FT_CONNECT);
    appendRemainingLength((unsigned int) (headerlen + payloadlen));        // Remaining Length
    appendVariableHeader(protocolName);                 // Protocol Name
    append(protocolVersion);                            // Protocol Version

    append(connectFlags);                               // Connect Flags
    appendShortValue(keepAliveSeconds);

    // Payload

    if (clientId.empty())
        throw Exception("Client Id shouldn't be empty");
    appendVariableHeader(clientId);                 // Client id

    if (!lastWillTopic.empty()) {
        appendVariableHeader(lastWillTopic);
        appendVariableHeader(willMessage);
    }

    if (!username.empty()) {
        appendVariableHeader(username);
        if (!password.empty())
            appendVariableHeader(password);
    }

    return *this;
}

const Buffer& MQTTFrame::setPUBLISH(const String& topic, const Buffer& data, MQTTQOS qos, bool dup, bool retain)
{
    m_type = FT_PUBLISH;
    bytes(0);

    auto packetlen = unsigned(topic.length() + 2 + data.bytes());
    if (qos != QOS_0)
        packetlen += 2; // Need extra two bytes of message id

    // Fixed header
    int dupFlag = dup ? 1 : 0;
    int retainFlag = retain ? 1 : 0;
    append((uint8_t) (FT_PUBLISH | (dupFlag<<3) | (qos << 1) | retainFlag));
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

void MQTTFrame::setACK(uint16_t messageId, MQTTFrameType ackType)
{
    bytes(0);

    int packetlen = 2;

    // Fixed header
    append((uint8_t) ackType);
    append((uint8_t) packetlen);                            // Remaining Length

    appendShortValue(messageId);
}

const Buffer& MQTTFrame::subscribeFrame(const String& topic, MQTTQOS qos)
{
    bytes(0);

    auto packetlen = unsigned((topic.length() + 2) + 2 + 1);

    // Fixed header
    append((uint8_t) (unsigned) (FT_SUBSCRIBE|(1 << 1)));
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

const Buffer& MQTTFrame::subscribeFrame(const Strings& topics, MQTTQOS qos)
{
    bytes(0);

    unsigned packetlen = 2;
    for (auto& topic: topics)
        packetlen += (topic.length() + 2) + 1;

    // Fixed header
    append((uint8_t) (unsigned) (FT_SUBSCRIBE|(1 << 1)));
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

const Buffer& MQTTFrame::unsubscribeFrame(const string& topic, MQTTQOS qos)
{
    bytes(0);

    unsigned packetlen = unsigned(topic.length() + 2) + 2;

    // Fixed header
    append((uint8_t) (unsigned) (FT_UNSUBSCRIBE|(1 << 1)));
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

const Buffer& MQTTFrame::unsubscribeFrame(const Strings& topics, MQTTQOS qos)
{
    bytes(0);

    unsigned packetlen = 2;
    for (auto& topic: topics)
        packetlen += (topic.length() + 2);

    // Fixed header
    append((uint8_t) (unsigned) (FT_UNSUBSCRIBE|(1 << 1)));
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

const Buffer& MQTTFrame::pingFrame()
{
    bytes(0);

    // Fixed header
    append((uint8_t) (unsigned) FT_PINGREQ);
    appendRemainingLength(0);

    return *this;
}

unsigned MQTTFrame::receiveRemainingLength(TCPSocket& connection)
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

MQTTFrameType MQTTFrame::ackType(MQTTFrameType messageType)
{
    MQTTFrameType _ackType;
    switch (messageType) {
        case FT_CONNECT:        _ackType = FT_CONNACK; break;
        case FT_PUBLISH:        _ackType = FT_PUBACK; break;
        case FT_SUBSCRIBE:      _ackType = FT_SUBACK; break;
        case FT_UNSUBSCRIBE:    _ackType = FT_UNSUBACK; break;
        default:                _ackType = FT_UNDEFINED; break;
    }
    return _ackType;
}

uint8_t MQTTFrame::readByte(TCPSocket& socket)
{
    uint8_t byte;
    socket.read((char*)&byte, 1);
    return byte;
}

uint16_t MQTTFrame::readShortInteger(TCPSocket& socket)
{
    uint16_t len;
    socket.read((char*)&len, 2);
    return ntohs(len);
}

void MQTTFrame::readString(TCPSocket& socket, String& header)
{
    uint16_t len = readShortInteger(socket);
    header.resize(len);
    socket.read((char*)header.c_str(), len);
}

void MQTTFrame::readConnectFrame(TCPSocket& socket, MQProtocol::Parameters& loginInfo)
{
    loginInfo.clear();

    readString(socket, loginInfo["protocol_name"]);
    loginInfo["protocol_version"] = to_string(readByte(socket));

    uint8_t connectFlags = readByte(socket);

    loginInfo["keep_alive"] = to_string(readShortInteger(socket));

    bool expectUsername = (connectFlags & CF_USERNAME) == CF_USERNAME;
    bool expectPassword = (connectFlags & CF_PASSWORD) == CF_PASSWORD;
    bool expectWillTopic = (connectFlags & CF_WILL_FLAG) == CF_WILL_FLAG;

    // Variable payload
    readString(socket, loginInfo["client_id"]);

    if (expectWillTopic) {
        readString(socket, loginInfo["will_topic"]);
        readString(socket, loginInfo["will_message"]);
    }

    if (expectUsername) {
        readString(socket, loginInfo["username"]);
        if (expectPassword)
            readString(socket, loginInfo["password"]);
    }
}

void MQTTFrame::readConnectACK(TCPSocket& connection)
{
    uint8_t connectACK;
    uint8_t connectRC;

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

void MQTTFrame::readPublishFrame(TCPSocket& connection, unsigned remainingLength, MQTTQOS qos, String& topicName)
{
    int variableHeaderLength = 0;

    readString(connection, topicName);
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
}

void MQTTFrame::readSubscribeFrame(TCPSocket& connection, unsigned remainingLength, MQTTQOS& qos, String& topicName)
{
    int variableHeaderLength = 0;

    uint16_t id;
    connection.read((char*)&id, 2);
    m_id = ntohs(id);
    variableHeaderLength += 2;

    unsigned dataLength = remainingLength - variableHeaderLength;

    Strings topicNames;
    qos = QOS_0;
    while (dataLength > 0) {
        String topic;
        readString(connection, topic);
        topicNames.push_back(topic);
        uint8_t aqos = readByte(connection);
        if (aqos > qos)
            qos = (MQTTQOS) aqos;
        dataLength -= topic.length() + 3;
    }
    topicName = topicNames.join(";");
}

void MQTTFrame::readUnknownFrame(TCPSocket& socket, unsigned remainingLength)
{
    checkSize(remainingLength + 1);
    socket.read(*this, remainingLength);
    bytes(remainingLength);
}

bool MQTTFrame::read(TCPSocket& socket, MQProtocol::Parameters& parameters, milliseconds timeout)
{
    if (!socket.readyToRead(timeout))
        throw ConnectionException("Read timeout");

    if (socket.socketBytes() == 0)
        throw ConnectionException("Connection closed");

    // Read fixed header
    uint8_t messageHeader;
    socket.read((char*)&messageHeader, 1);
    m_type = (MQTTFrameType) (messageHeader & 0xF0);
    m_qos = (MQTTQOS) ((messageHeader & 0x06) >> 1);
    unsigned remainingLength = receiveRemainingLength(socket);

    switch (m_type) {
        case FT_CONNECT:
            readConnectFrame(socket, parameters);
            break;

        case FT_CONNACK:
            readConnectACK(socket);
            break;

        case FT_PUBLISH:
            readPublishFrame(socket, remainingLength, m_qos, parameters["destination"]);
            break;

        case FT_SUBSCRIBE:
            readSubscribeFrame(socket, remainingLength, m_qos, parameters["destination"]);
            break;

        case FT_UNDEFINED:
            throw Exception("Received frame type");

        default:
            readUnknownFrame(socket, remainingLength);
            break;
    }

    return true;
}

std::string MQTTFrame::typeName() const
{
    switch (m_type) {
        case FT_UNDEFINED:  return "UNDEFINED";
        case FT_CONNECT:    return "CONNECT";
        case FT_CONNACK:    return "CONN ACK";
        case FT_PUBLISH:    return "PUBLISH";
        case FT_PUBACK:     return "PUB ACK";
        case FT_PUBREC:     return "PUB REC";
        case FT_PUBREL:     return "PUB REL";
        case FT_PUBCOMP:    return "PUB COMP";
        case FT_SUBSCRIBE:  return "SUBSCRIBE";
        case FT_SUBACK:     return "SUB ACK";
        case FT_UNSUBSCRIBE: return "UNSUBSCRIBE";
        case FT_UNSUBACK:   return "UNSUB ACK";
        case FT_PINGREQ:    return "PING REQ";
        case FT_PINGRESP:   return "PING RESP";
        case FT_DISCONNECT: return "DISCONNECT";
        default:            break;
    }
    return "UNDEFINED";
}
