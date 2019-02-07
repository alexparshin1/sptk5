/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       MQTTFrame.cpp - description                            ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Sunday December 23 2018                                ║
║  copyright            © 1999-2019 by Alexey Parshin. All rights reserved.    ║
║  email                alexeyp@gmail.com                                      ║
╚══════════════════════════════════════════════════════════════════════════════╝
┌──────────────────────────────────────────────────────────────────────────────┐
│   This library is free software; you can redistribute it and/or modify it    │
│   under the terms of the GNU Library General Public License as published by  │
│   the Free Software Foundation; either version 2 of the License, or (at your │
│   option) any later version.                                                 │
│                                                                              │
│   This library is distributed in the hope that it will be useful, but        │
│   WITHOUT ANY WARRANTY; without even the implied warranty of                 │
│   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library   │
│   General Public License for more details.                                   │
│                                                                              │
│   You should have received a copy of the GNU Library General Public License  │
│   along with this library; if not, write to the Free Software Foundation,    │
│   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#ifndef __MQTT_FRAME_H__
#define __MQTT_FRAME_H__

#include <sptk5/cnet>
#include "MQProtocol.h"

namespace sptk {

/**
 * MQTT Protocol versions
 */
enum MQTTProtocolVersion : uint8_t
{
    MQTT_PROTOCOL_V31  = 3,
    MQTT_PROTOCOL_V311 = 4
};

/**
 * QOS (Quality Of Service) levels
 */
enum MQTTQOS : uint8_t
{
    QOS_0               = 0,
    QOS_1               = 1,
    QOS_2               = 2
};

/**
 * MQTT frame types
 */
enum MQTTFrameType : uint8_t
{
    FT_UNDEFINED        = 0,
    FT_CONNECT          = 0x10,
    FT_CONNACK          = 0x20,
    FT_PUBLISH          = 0x30,
    FT_PUBACK           = 0x40,
    FT_PUBREC           = 0x50,
    FT_PUBREL           = 0x60,
    FT_PUBCOMP          = 0x70,
    FT_SUBSCRIBE        = 0x80,
    FT_SUBACK           = 0x90,
    FT_UNSUBSCRIBE      = 0xA0,
    FT_UNSUBACK         = 0xB0,
    FT_PINGREQ          = 0xC0,
    FT_PINGRESP         = 0xD0,
    FT_DISCONNECT       = 0xE0
};

/**
 * MQTT connect flags for CONNECT message
 */
enum MQTTConnectFlags : uint8_t
{
    CF_WILL_QOS_0       = 0x0,
    CF_CLEAN_SESSION    = 0x2,
    CF_WILL_FLAG        = 0x4,
    CF_WILL_QOS_1       = 0x8,
    CF_WILL_QOS_2       = 0x10,
    CF_WILL_RETAIN      = 0x20,
    CF_PASSWORD         = 0x40,
    CF_USERNAME         = 0x80
};

/**
 * MQTT frame
 */
class MQTTFrame : public Buffer
{
    /**
     * Message type
     */
    MQTTFrameType           m_type;

    /**
     * Message id
     */
    uint16_t                m_id;

    /**
     * QOS (Quality Of Service)
     */
    MQTTQOS                 m_qos;

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
     * @param socket TCPSocket&, MQTT server connection
     * @returns 16 bit value
     */
    uint8_t readByte(TCPSocket& socket);

    /**
     * Receive 16-bit integer and swaps its bytes
     * @param socket TCPSocket&, MQTT server connection
     * @returns 16 bit value
     */
    uint16_t readShortInteger(TCPSocket& socket);

    /**
     * Receive character string as 16 bit length (Big Endian) followed by the string characters
     * @param socket Socket&, MQTT server connection
     * @param header string, header (output)
     */
    void readString(TCPSocket& socket, String& header);

    /**
     * Receive 'remaining length' using the multi-byte presentation MQTT algorithm
     * @param connection Socket&, MQTT server connection
     */
    unsigned receiveRemainingLength(TCPSocket& connection);

    /**
     * Receive CONNECT MQTT frame (connection request)
     * @param socket Socket&, MQTT server connection
     */
    void readConnectFrame(TCPSocket& socket, MQProtocol::Parameters& loginInfo);

    /**
     * Receive CONNACK MQTT frame
     * @param connection Socket&, MQTT server connection
     */
    void readConnectACK(TCPSocket& connection);

    /**
     * Receive any MQTT frame, starting from variable header, placing the payload into this object
     * @param connection Socket&, MQTT server connection
     * @param remainingLength unsigned, Remaining length
     */
    void readUnknownFrame(TCPSocket& connection, unsigned remainingLength);

    /**
     * Receive PUBLISH MQTT frame, starting from variable header, placing the payload into this object
     * @param connection TCPSocket&, MQTT server connection
     * @param remainingLength unsigned, Remaining length
     * @param qos QOS, Quality Of Service
     * @param topicName std::string*, Related topic if applicable (output)
     */
    void readPublishFrame(TCPSocket& connection, unsigned remainingLength, MQTTQOS qos, String& topicName);

public:
    /**
     * Constructor
     * @param type MessageType, MQTT frame type
     * @param id uint16_t, Message id
     * @param qos QOS, Message QOS (Quality Of Service)
     */
    MQTTFrame(MQTTFrameType type, uint16_t id, MQTTQOS qos);

    /**
     * Get MQTT frame type
     * @return MQTT frame type
     */
    MQTTFrameType type() const
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
    MQTTQOS qos() const
    {
        return m_qos;
    }

    /**
     * Set frame to ACK
     * @param messageId         Message id
     * @param ackType           ACK type, one of { MT_CONNACK..MT_PUBCOMP }
     */
    void setACK(uint16_t messageId, MQTTFrameType ackType);

    /**
     * Generate MQTT CONNECT frame
     * @param keepAliveSeconds uint16_t, Interval between keep alive packets, seconds
     * @param username std::string, Optional user name or empty string if not needed
     * @param password std::string, Optional password or empty string if not needed
     * @param clientId std::string, Client id
     * @param willTopic std::string, Optional topic to send last will
     * @param protocolVersion MQTTProtocolVersion, Optional MQTT protocol version
     * @return this object reference
     */
    const Buffer& setCONNECT(uint16_t keepAliveSeconds, String username, String password, String clientId,
                             String willTopic, MQTTProtocolVersion protocolVersion);

    /**
     * Generate MQTT PUBLISH frame
     * @param topic std::string, Topic name to publish to
     * @param data const Buffer&, Data to publish
     * @param qos QOS, QOS (Quality of Service)
     * @param dup bool, Flag: this packet is a duplicate
     * @param retain bool, Flag: retain this message
     * @return this object reference
     */
    const Buffer& publishFrame(const std::string& topic, const Buffer& data, MQTTQOS qos, bool dup, bool retain);

    /**
     * Generate MQTT SUBSCRIBE frame
     * @param topic std::string, Topic name to publish to
     * @param qos QOS, QOS (Quality of Service)
     * @return this object reference
     */
    const Buffer& subscribeFrame(const std::string& topic, MQTTQOS qos);

    /**
     * Generate MQTT SUBSCRIBE frame
     * @param topics const Strings&, Topic names to publish to
     * @param qos QOS, QOS (Quality of Service)
     * @return this object reference
     */
    const Buffer& subscribeFrame(const Strings& topics, MQTTQOS qos);

    /**
     * Generate MQTT UNSUBSCRIBE frame
     * @param topic std::string, Topic name to unsubscribe from
     * @param qos QOS, QOS (Quality of Service)
     * @return this object reference
     */
    const Buffer& unsubscribeFrame(const std::string& topic, MQTTQOS qos);

    /**
     * Generate MQTT UNSUBSCRIBE frame
     * @param topic const Strings&, Topic names to unsubscribe from
     * @param qos QOS, QOS (Quality of Service)
     * @return this object reference
     */
    const Buffer& unsubscribeFrame(const Strings& topics, MQTTQOS qos);

    /**
     * Generate MQTT PING frame
     * @return this object reference
     */
    const Buffer& pingFrame();

    /**
     * Receive next MQTT frame
     * @param socket TCPSocket&, MQTT server connection
     * @param timeout uint32_t, Receive timeout, milliseconds
     * @param headers std::string&, Related topic name if applicable
     * @return 0 if failure
     */
    bool read(TCPSocket& socket, MQProtocol::Parameters& headers, std::chrono::milliseconds timeout);

    static MQTTFrameType ackType(MQTTFrameType messageType);
};

} // namespace sptk

#endif
