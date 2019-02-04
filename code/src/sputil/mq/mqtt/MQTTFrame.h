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

#ifndef __MQTT_FRAME_H__
#define __MQTT_FRAME_H__

#include "common/Buffer.h"
#include "common/MQClient.h"

namespace vsparc { namespace mqtt {

/**
 * MQTT Protocol versions
 */
enum MQTTProtocol {
    MQTT_PROTOCOL_V31  = 1,
    MQTT_PROTOCOL_V311 = 2
};

/**
 * MQTT Message types
 */
enum MessageType {
    MT_UNDEFINED = 0,
    MT_CONNECT = 0x10,
    MT_CONNACK = 0x20,
    MT_PUBLISH = 0x30,
    MT_PUBACK = 0x40,
    MT_PUBREC = 0x50,
    MT_PUBREL = 0x60,
    MT_PUBCOMP = 0x70,
    MT_SUBSCRIBE = 0x80,
    MT_SUBACK = 0x90,
    MT_UNSUBSCRIBE = 0xA0,
    MT_UNSUBACK = 0xB0,
    MT_PINGREQ = 0xC0,
    MT_PINGRESP = 0xD0,
    MT_DISCONNECT = 0xE0
};

/**
 * MQTT connect flags for CONNECT message
 */
enum ConnectFlags {
    CF_USERNAME = 0x80,
    CF_PASSWORD = 0x40,
    CF_WILL_RETAIN = 0x20,
    CF_WILL_QOS_0 = 0x0,
    CF_WILL_QOS_1 = 0x8,
    CF_WILL_QOS_2 = 0x10,
    CF_WILL_FLAG = 0x4,
    CF_CLEAN_SESSION = 0x2
};

/**
 * QOS (Quality Of Service) levels
 */
enum QOS {
    QOS_0   = 0,
    QOS_1   = 1,
    QOS_2   = 2
};

/**
 * MQTT frame (packet)
 */
class Frame : public Buffer
{
    /**
     * Message type
     */
    MessageType         m_type;

    /**
     * Message id
     */
    uint16_t            m_id;

    /**
     * QOS (Quality Of Service)
     */
    QOS                 m_qos;

    /**
     * Next packet id generator
     * @returns next packet id
     */
    static uint16_t nextPacketId();

    /**
     * Append short (16 bit) as Big Endian to the frame buffer
     * @param value     16-bit integer
     */
    void appendShortValue(uint16_t value);

    /**
     * Append string as 16 bit length (Big Endian) followed by the string characters
     * @param data const char*, Character string
     * @param dataLength uint16_t, Character string length
     */
    void appendVariableHeader(const char* data, uint16_t dataLength);

    /**
     * Append string as 16 bit length (Big Endian) followed by the string characters
     * @param data const std::string&, Character string
     */
    void appendVariableHeader(const std::string& data);

    /**
     * Append 'remaining length' using the multi-byte presentation MQTT algorithm
     * @param remainingLength unsigned, Remaining length
     */
    void appendRemainingLength(unsigned remainingLength);

    /**
     * Receive 16-bit integer and swaps its bytes
     * @param connection Socket&, MQTT server connection
     * @returns 16 bit value
     */
    uint16_t receiveShortValue(Socket& connection);

    /**
     * Receive character string as 16 bit length (Big Endian) followed by the string characters
     * @param connection Socket&, MQTT server connection
     * @param header string, header (output)
     */
    void receiveVariableHeader(Socket& connection, std::string& header);

    /**
     * Receive 'remaining length' using the multi-byte presentation MQTT algorithm
     * @param connection Socket&, MQTT server connection
     */
    unsigned receiveRemainingLength(Socket& connection);

    /**
     * Receive CONNACK MQTT frame
     * @param connection Socket&, MQTT server connection
     */
    void receiveConnectACK(Socket& connection);

    /**
     * Receive any MQTT frame, starting from variable header, placing the payload into this object
     * @param connection Socket&, MQTT server connection
     * @param remainingLength unsigned, Remaining length
     */
    void receiveAnyFrame(Socket& connection, unsigned remainingLength);

    /**
     * Receive PUBLISH MQTT frame, starting from variable header, placing the payload into this object
     * @param connection Socket&, MQTT server connection
     * @param remainingLength unsigned, Remaining length
     * @param qos QOS, Quality Of Service
     * @param topicName std::string*, Related topic if applicable (output)
     */
    void receivePublishFrame(Socket& connection, unsigned remainingLength, QOS qos, std::string& topicName);

public:
    /**
     * Constructor
     * @param type MessageType, MQTT frame type
     * @param id uint16_t, Message id
     * @param qos QOS, Message QOS (Quality Of Service)
     */
    Frame(MessageType type, uint16_t id, QOS qos);

    /**
     * Get MQTT frame type
     * @return MQTT frame type
     */
    MessageType type() const
    {
        return m_type;
    }

    /**
     * Get MQTT frame type name
     * @return MQTT frame type name
     */
    std::string typeName() const;

    /**
     * Get message id
     */
    uint16_t id() const
    {
        return m_id;
    }

    /**
     * Get message QOS
     */
    QOS qos() const
    {
        return m_qos;
    }

    /**
     * Generate MQTT CONNECT frame
     * @param keepAliveSeconds uint16_t, Interval between keep alive packets, seconds
     * @param userName std::string, Optional user name or empty string if not needed
     * @param password std::string, Optional password or empty string if not needed
     * @param clientId std::string, Client id
     * @param willTopic std::string, Optional topic to send last will
     * @param protocol MQTTProtocol, Optional MQTT protocol version
     * @param host const Host&, MQTT broker host - for the will message content
     * @return this object reference
     */
    const Buffer& connectFrame(uint16_t keepAliveSeconds, std::string userName, std::string password, std::string clientId,
                                   std::string willTopic, MQTTProtocol protocol, const Host& host);

    /**
     * Generate MQTT PUBLISH frame
     * @param topic std::string, Topic name to publish to
     * @param data const Buffer&, Data to publish
     * @param qos QOS, QOS (Quality of Service)
     * @param dup bool, Flag: this packet is a duplicate
     * @param retain bool, Flag: retain this message
     * @return this object reference
     */
    const Buffer& publishFrame(const std::string& topic, const Buffer& data, QOS qos, bool dup, bool retain);

    /**
     * Generate MQTT PUBACK frame
     * @param messageId uint16_t, Message id
     * @param ackType MessageType, ACK type, one of { MT_PUBACK, MT_PUBREC, MT_PUBREL, MT_PUBCOMP }
     * @return this object reference
     */
    const Buffer& publishAckFrame(uint16_t messageId, MessageType ackType);

    /**
     * Generate MQTT SUBSCRIBE frame
     * @param topic std::string, Topic name to publish to
     * @param qos QOS, QOS (Quality of Service)
     * @return this object reference
     */
    const Buffer& subscribeFrame(const std::string& topic, QOS qos);

    /**
     * Generate MQTT SUBSCRIBE frame
     * @param topics const Strings&, Topic names to publish to
     * @param qos QOS, QOS (Quality of Service)
     * @return this object reference
     */
    const Buffer& subscribeFrame(const Strings& topics, QOS qos);

    /**
     * Generate MQTT UNSUBSCRIBE frame
     * @param topic std::string, Topic name to unsubscribe from
     * @param qos QOS, QOS (Quality of Service)
     * @return this object reference
     */
    const Buffer& unsubscribeFrame(const std::string& topic, QOS qos);

    /**
     * Generate MQTT UNSUBSCRIBE frame
     * @param topic const Strings&, Topic names to unsubscribe from
     * @param qos QOS, QOS (Quality of Service)
     * @return this object reference
     */
    const Buffer& unsubscribeFrame(const Strings& topics, QOS qos);

    /**
     * Generate MQTT PING frame
     * @return this object reference
     */
    const Buffer& pingFrame();

    /**
     * Receive next MQTT frame
     * @param socket Socket&, MQTT server connection
     * @param timeoutMS uint32_t, Receive timeout, milliseconds
     * @param topicName std::string&, Related topic name if applicable
     * @return 0 if failure
     */
    int receive(Socket& socket, uint32_t timeoutMS, std::string& topicName);

    static MessageType ackType(MessageType messageType);
};

} // namespace mqtt
} // namespace vsparc

#endif
