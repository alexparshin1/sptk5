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
class SP_EXPORT MQTTFrame : public Buffer
{
    MQTTFrameType           m_type;     ///< Frame type
    uint16_t                m_id;       ///< Frame id
    QOS                     m_qos;      ///< QOS (Quality Of Service)

    /**
     * Next packet id generator
     * @returns next packet id
     */
    static uint16_t nextPacketId();

    /**
     * Append short (16 bit) as Big Endian to the frame buffer
     * @param value             16-bit integer
     */
    void appendShortValue(uint16_t value);

    /**
     * Append string as 16 bit length (Big Endian) followed by the string characters
     * @param data              Character string
     * @param dataLength        Character string length
     */
    void appendVariableHeader(const char* data, uint16_t dataLength);

    /**
     * Append string as 16 bit length (Big Endian) followed by the string characters
     * @param data              Character string
     */
    void appendVariableHeader(const String& data);

    /**
     * Append 'remaining length' using the multi-byte presentation MQTT algorithm
     * @param remainingLength   Remaining length
     */
    void appendRemainingLength(unsigned remainingLength);

    /**
     * Receive 16-bit integer and swaps its bytes
     * @param socket            MQTT server connection
     * @returns 16 bit value
     */
    uint8_t readByte(TCPSocket& socket);

    /**
     * Receive 16-bit integer and swaps its bytes
     * @param socket            MQTT server connection
     * @returns 16 bit value
     */
    uint16_t readShortInteger(TCPSocket& socket);

    /**
     * Receive character string as 16 bit length (Big Endian) followed by the string characters
     * @param socket            MQTT server connection
     * @param header            Header (output)
     */
    void readString(TCPSocket& socket, String& header);

    /**
     * Receive 'remaining length' using the multi-byte presentation MQTT algorithm
     * @param connection        MQTT server connection
     */
    unsigned receiveRemainingLength(TCPSocket& connection);

    /**
     * Receive CONNECT MQTT frame (connection request)
     * @param socket            MQTT server connection
     */
    void readConnectFrame(TCPSocket& socket, MQProtocol::Parameters& loginInfo);

    /**
     * Receive CONNACK MQTT frame
     * @param connection        MQTT server connection
     */
    void readConnectACK(TCPSocket& connection);

    /**
     * Receive any MQTT frame, starting from variable header, placing the payload into this object
     * @param connection        MQTT server connection
     * @param remainingLength   Remaining length
     */
    void readUnknownFrame(TCPSocket& connection, unsigned remainingLength);

    /**
     * Receive PUBLISH frame, starting from variable header, placing the payload into this object
     * @param connection        MQTT server connection
     * @param remainingLength   Remaining length
     * @param qos               QOS - Quality Of Service
     * @param topicName         Related topic if applicable (output)
     */
    void readPublishFrame(TCPSocket& connection, unsigned remainingLength, QOS qos, String& topicName);

    /**
     * Receive SUBSCRIBE frame
     * @param connection        MQTT server connection
     * @param remainingLength   Remaining length
     * @param qos               QOS - Quality Of Service
     * @param topicName         Topic if applicable (output)
     */
    void readSubscribeFrame(TCPSocket& connection, unsigned remainingLength, QOS& qos, String& topicName);

public:
    /**
     * Constructor
     * @param type              MQTT frame type
     * @param id                Message id
     * @param qos               QOS - Quality Of Service
     */
    explicit MQTTFrame(MQTTFrameType type=FT_UNDEFINED, uint16_t id=0, QOS qos=QOS_0);

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
    QOS qos() const
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
     * @param keepAliveSeconds  Interval between keep alive packets, seconds
     * @param username          Optional user name or empty string if not needed
     * @param password          Optional password or empty string if not needed
     * @param clientId          Client id
     * @param lastWillTopic     Optional topic to send last will
     * @param lastWillMessage   Optional last will message
     * @param protocolVersion   Optional MQTT protocol version
     * @return this object reference
     */
    const Buffer& setCONNECT(uint16_t keepAliveSeconds, const String& username, const String& password,
                             const String& clientId,
                             const String& lastWillTopic, const String& lastWillMessage,
                             MQTTProtocolVersion protocolVersion);

    /**
     * Generate MQTT PUBLISH frame
     * @param topic             Destination topic or queue name
     * @param data              Data to publish
     * @param qos               QOS - Quality of Service
     * @param dup               Duplicate packet flag
     * @param retain            Retain message flag
     * @return this object reference
     */
    const Buffer& setPUBLISH(const String& topic, const Buffer& data, QOS qos=QOS_0, bool dup=false, bool retain=false);

    /**
     * Generate MQTT SUBSCRIBE frame
     * @param topic             Destination topic or queu name
     * @param qos               QOS - Quality of Service
     * @return this object reference
     */
    const Buffer& setSUBSCRIBE(const String& topic, QOS qos);

    /**
     * Generate MQTT UNSUBSCRIBE frame
     * @param topic             Topic name to unsubscribe from
     * @param qos               QOS - Quality of Service
     * @return this object reference
     */
    const Buffer& unsubscribeFrame(const std::string& topic, QOS qos);

    /**
     * Generate MQTT UNSUBSCRIBE frame
     * @param topic             Topic name to unsubscribe from
     * @param qos               QOS - Quality of Service
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
     * @param socket            MQTT server connection
     * @param destination       Message destination (output)
     * @param parameters        Message headers (output)
     * @param timeout           Receive timeout, milliseconds
     * @return 0 if failure
     */
    bool read(TCPSocket& socket, String& destination, MQProtocol::Parameters& parameters,
              std::chrono::milliseconds timeout);

    /**
     * Get ACK type corresponding to source message type
     * @param messageType       Source message type
     * @return ACK type
     */
    static MQTTFrameType ackType(MQTTFrameType messageType);
};

} // namespace sptk

#endif
