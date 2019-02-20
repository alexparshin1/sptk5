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

#ifndef __MQTT_CLIENT_H__
#define __MQTT_CLIENT_H__

#include "mqtt/MQTTFrame.h"

namespace vsparc { namespace mqtt {

/**
 * MQTT MQ client
 *
 * Provides methods to connect to MQ server, send and receive STOMP frames, etc.
 */
class Client : public MQClient
{
    /**
     * MQ server connection socket
     */
    TCPSocket*          m_mqSocket;

    /**
     * Socket read buffer
     */
    Buffer              m_readBuffer;

    /**
     * SSL connection context
     * Use loadSslKeys() method prior to attempting using SSL-encrypted connect()
     */
    SSLContext          m_sslContext;

    /**
     * Mutex protecting read-write from async connect-disconnect
     */
    std::mutex          m_mutex;

    /**
     * Topic where last will message is published if connection is interrupted ungracefully
     */
    const std::string   m_lastWillTopic;

protected:

    /*
     * Connect to MQ server, optionally using username and password
     * @param user std::string, User name
     * @param password std::string, Password
     * @param port uint16_t, MQ server port number
     * @param encrypted bool, If true then use SSL encrypted connection
     * @param localIpAddress const char*, If not NULL then socket connection binds to provided local IP address
     * @param timeoutSeconds size_t, Single server connection timeout. If connection attempt gets a timeout, it switches to the next server in the list of servers (see constructor)
     */
    virtual void _connect(const std::string& user, const std::string& password, bool encrypted,
                          const char* localIpAddress,
                          size_t timeoutSeconds);

    /**
     * Load SSL private key and certificate(s).
     *
     * This method should be used before the first attempt to use connect() method with encrypted=true.
     * Call of this method doesn't affect non-encrypted connections.
     * Private key and certificates must be encoded with PEM format.
     * A single file containing private key and certificate can be used by supplying it for both,
     * private key and certificate parameters.
     * If private key is protected with password, then password can be supplied to auto-answer.
     * @param keyFileName std::string, Private key file name
     * @param certificateFileName std::string, Certificate file name
     * @param password std::string, Key file password
     * @param caFileName std::string, optional CA (root certificate) file name
     * @param verifyMode int, ether SSL_VERIFY_NONE, or SSL_VERIFY_PEER, for server can be ored with SSL_VERIFY_FAIL_IF_NO_PEER_CERT and/or SSL_VERIFY_CLIENT_ONCE
     * @param verifyDepth int, Connection verify depth
     */
    virtual void _loadSslKeys(const std::string& keyFileName, const std::string& certificateFileName,
                              const std::string& password, const std::string& caFileName = "",
                              int verifyMode = SSL_VERIFY_NONE, int verifyDepth = 0);

    /**
     * Send/receive messages to server until m_terminated is set to false.
     * Executed after client connected to MQ server.
     * @param singleMessage bool, If true then exit after getting a single message
     */
    virtual void _messageLoop(bool singleMessage);

    /**
     * Disconnects from MQ server
     */
    virtual void _disconnect(bool polite);

    /**
     * Subscribe to queue
     * @param queueName std::string, Queue name
     * @param waitForACK bool, Automatic message acknowledgement. If true then call is blocked until ACK is received, or timeout.
     * @param waitTimeoutMS uint32_t, Wait for ACK timeout, milliseconds. Only used if waitForACK is true.
     */
    virtual void _subscribeToQueue(const std::string& queueName, bool waitForACK, uint32_t waitTimeoutMS);

    /**
     * Unsubscribe from queue
     * @param queueName std::string, queue name
     * @param waitForACK bool, Automatic message acknowledgement. If true then call is blocked until ACK is received, or timeout.
     * @param waitTimeoutMS uint32_t, Wait for ACK timeout, milliseconds. Only used if waitForACK is true.
     */
    virtual void _unsubscribeFromQueue(const std::string& queueName, bool waitForACK, uint32_t waitTimeoutMS);

    /**
     * Subscribe to topic
     * @param topicName std::string, topic name
     * @param waitForACK bool, Automatic message acknowledgement. If true then call is blocked until ACK is received, or timeout.
     * @param waitTimeoutMS uint32_t, Wait for ACK timeout, milliseconds. Only used if waitForACK is true.
     */
    virtual void _subscribeToTopic(const std::string& topicName, bool waitForACK, uint32_t waitTimeoutMS);

    /**
     * Unsubscribe from topic
     * @param topicName std::string, topic name
     * @param waitForACK bool, Automatic message acknowledgement. If true then call is blocked until ACK is received, or timeout.
     * @param waitTimeoutMS uint32_t, Wait for ACK timeout, milliseconds. Only used if waitForACK is true.
     */
    virtual void _unsubscribeFromTopic(const std::string& topicName, bool waitForACK, uint32_t waitTimeoutMS);

    /**
     * Send MQTT frame to MQ server
     * @param frame const CMQTTFrame&, MQTT frame to send
     * @param waitForACK bool, Automatic message acknowledgement. If true then call is blocked until ACK is received, or timeout.
     * @param waitTimeoutMS uint32_t, Wait for ACK timeout, milliseconds. Only used if waitForACK is true.
     */
    void send(const Frame& frame, bool waitForACK, uint32_t waitTimeoutMS);

    /**
     * Send message to queue or topic
     * @param destination std::string, Full queue or topic name
     * @param message const MQMessage&, Message
     * @param data const Buffer&, Message data
     * @param waitForACK bool, Automatic message acknowledgement. If true then call is blocked until ACK is received, or timeout.
     * @param waitTimeoutMS uint32_t, Wait for ACK timeout, milliseconds. Only used if waitForACK is true.
     */
    void sendMessage(std::string destination, const MQMessage& message, bool waitForACK, uint32_t waitTimeoutMS);

    /**
     * Subscribe to a destination (queue or topic)
     * @param destination std::string, Subscribe destination: full name of the queue or topic
     * @param waitForACK bool, Automatic message acknowledgement. If true then call is blocked until ACK is received, or timeout.
     * @param waitTimeoutMS uint32_t, Wait for ACK timeout, milliseconds. Only used if waitForACK is true.
     */
    void subscribe(std::string destination, bool waitForACK, uint32_t waitTimeoutMS);

    /**
     * Subscribe to a list of destinations (queues or topics)
     * @param destinations const Strings&, Subscribe destination: full name of the queue or topic
     * @param waitForACK bool, Automatic message acknowledgement. If true then call is blocked until ACK is received, or timeout.
     * @param waitTimeoutMS uint32_t, Wait for ACK timeout, milliseconds. Only used if waitForACK is true.
     */
    void subscribe(const Strings& destinations, bool waitForACK, uint32_t waitTimeoutMS);

    /**
     * Unsubscribe from a destination (queue or topic)
     * @param destination std::string, Subscribe destination: full name of the queue or topic
     * @param waitForACK bool, Automatic message acknowledgement. If true then call is blocked until ACK is received, or timeout.
     * @param waitTimeoutMS uint32_t, Wait for ACK timeout, milliseconds. Only used if waitForACK is true.
     */
    void unsubscribe(std::string destination, bool waitForACK, uint32_t waitTimeoutMS);

    /**
     * Unsubscribe from a list of destinations (queues or topics)
     * @param destinations const Strings&, Subscribe destination: full name of the queue or topic
     * @param waitForACK bool, Automatic message acknowledgement. If true then call is blocked until ACK is received, or timeout.
     * @param waitTimeoutMS uint32_t, Wait for ACK timeout, milliseconds. Only used if waitForACK is true.
     */
    void unsubscribe(const Strings& destinations, bool waitForACK, uint32_t waitTimeoutMS);

    /**
     * Receive a message from MQ server.
     * Blocks if frame is not available or incomplete.
     * @param frame Frame&, Received MQTT frame
     * @param timeoutMS size_t, Receive timeout, milliseconds
     * @param topicName std::string&, Topic name if applicable (output)
     * @returns true if MQTT frame is received
     */
    bool receiveFrame(Frame& frame, uint32_t timeoutMS, std::string& topicName);

public:

    /**
     * Constructor
     *
     * Creates MQTT client object not yet connected to server.
     * @param socketGroup SocketGroup&, Socket group managing client's socket events.
     * @param hosts MQServers&, MQ server hosts.
     * @param clientId std::string, Unique client id. If empty string then generated automatically.
     * @param lastWillTopic std::string, Topic where client publishes a message if connection is interrupted ungracefully. Ignored if empty string.
     */
    Client(SocketGroup& socketGroup, MQServers& hosts, const std::string& clientId,
           const std::string& lastWillTopic = "");

    /**
     * Destructor
     */
    virtual ~Client();

    //------------------------------------------------------------------
    //              High-level MQTT commands
    //------------------------------------------------------------------

    /**
     * Send message to queue
     * @param queueName std::string, Queue name
     * @param message const MQMessage&, Message
     * @param data const Buffer&, Message data
     * @param waitForACK bool, Automatic message acknowledgement. If true then call is blocked until ACK is received, or timeout.
     * @param waitTimeoutMS uint32_t, Wait for ACK timeout, milliseconds. Only used if waitForACK is true.
     */
    virtual void sendToQueue(const std::string& queueName, const MQMessage& message, bool waitForACK,
                             uint32_t waitTimeoutMS);

    /**
     * Send message to topic
     * @param topicName std::string, Topic name
     * @param message const MQMessage&, Message
     * @param waitForACK bool, Automatic message acknowledgement. If true then call is blocked until ACK is received, or timeout.
     * @param waitTimeoutMS uint32_t, Wait for ACK timeout, milliseconds. Only used if waitForACK is true.
     */
    virtual void sendToTopic(std::string topicName, const MQMessage& message, bool waitForACK, uint32_t waitTimeoutMS);

    /**
     * Subscribe to a list of topics
     * @param topicNames const Strings&, topic names
     * @param waitTimeoutMS uint32_t, Wait for ACK timeout, milliseconds.
     */
    virtual void subscribeToTopics(const Strings& topicNames, uint32_t waitTimeoutMS=30000);

    /**
     * Subscribe to a list of topics
     * @param topicNames const Strings&, topic names
     * @param waitTimeoutMS uint32_t, Wait for ACK timeout, milliseconds.
     */
    virtual void unsubscribeFromTopics(const Strings& topicNames, uint32_t waitTimeoutMS=30000);

    /**
     * Send ACK command MQ server, confirming message consumed.
     * @param messageId std::string, Related message id
     * @param transactionId std::string, Transaction id, if message was sent in a transaction
     * @param waitForACK bool, Automatic message acknowledgement. If true then call is blocked until ACK is received, or timeout.
     * @param waitTimeoutMS uint32_t, Wait for ACK timeout, milliseconds. Only used if waitForACK is true.
     */
    virtual void ack(const std::string& messageId, const std::string& transactionId = "");

    /**
     * Send NACK command MQ server, not confirming message consumed.
     * Message is returned to the queue, and server will try re-delivering it to any client.
     * @param messageId std::string, Related message id
     * @param transactionId std::string, Transaction id, if message was sent in a transaction
     */
    virtual void nack(const std::string& messageId, const std::string& transactionId = "");

    /**
     * Send PING command to MQ server.
     */
    virtual void ping();

    bool connected() OVERRIDE;
};

}} // namespace vsparc

#endif
