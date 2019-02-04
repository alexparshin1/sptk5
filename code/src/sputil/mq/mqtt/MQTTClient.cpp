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

#include "mqtt/MQTTClient.h"

#ifndef _WIN32
#include <netinet/tcp.h>

#endif

using namespace std;
using namespace vsparc;
using namespace vsparc::mqtt;

// Define _DEBUG_ if you need debugging printout
//#define _DEBUG_
#ifdef _DEBUG_
    void debug(const std::string& message)
    {
        cout << DateTime::Now().timeString(true) << " " << message << endl;
    }
#else
    #define debug(msg)
#endif

const int pingIntervalMS = 30000;

// Initializes MQTT client
Client::Client(SocketGroup& socketGroup, MQServers& hosts, const string& clientId, const string& lastWillTopic)
: MQClient(socketGroup, hosts, clientId), m_mqSocket(nullptr), m_lastWillTopic(lastWillTopic)
{
    m_readBuffer.checkSize(16384);
}

Client::~Client()
{
    disconnect(false);

    lock_guard<mutex>   lock(m_mutex);
    delete m_mqSocket;
}

// Connects to MQ server
void Client::_connect(const string& user, const string& password, bool encrypted, const char* localIpAddress,
                      size_t timeoutSeconds)
{
    if (m_hosts.empty())
        throw Exception("List of MQ servers is empty");

    Host host = m_hosts.current();

    // TCP connection
    string error;
    size_t hostTimeoutSeconds = timeoutSeconds / m_hosts.size();
    for (size_t attempt = 0; attempt < m_hosts.size(); attempt++) {
        try {
            lock_guard<mutex>   lock(m_mutex);
            if (encrypted)
                m_mqSocket = new SSLSocket(m_sslContext);
            else
                m_mqSocket = new TCPSocket;
            debug("Connecting to MQTT broker " + m_mqSocket->host());
            m_mqSocket->connect(host.address(), hostTimeoutSeconds, localIpAddress);
            cout << "Connected to MQTT broker " << m_mqSocket->host() << ":" << m_mqSocket->port() << "." << endl;
            error = "";
            break;
        }
        catch (const Exception& e) {
            m_mqSocket->disconnect();
            delete m_mqSocket;
            m_mqSocket = nullptr;
            stringstream err;
            err << "Can't connect to broker " << host.host() << ":" << host.port() << ". " << e.what();
            error = err.str();
            sleep(1);
            host = m_hosts.next();
        }
    }

    if (!error.empty())
        throw Exception(error);

    m_mqSocket->setOption(IPPROTO_TCP, TCP_NODELAY, 1);

    // Create and send CONNECT frame
    Frame frame(MT_CONNECT, 0, QOS_1);
    frame.connectFrame(600, user, password, m_clientId, m_lastWillTopic,
                       MQTT_PROTOCOL_V31, host);
    send(frame, false, 0);

    string stub;
    try {
        if (!receiveFrame(frame, 10000, stub)) {
            m_mqSocket->disconnect();
            throw TimeoutException("Connection timeout");
        }
    }
    catch(...) {
        THROW_EXCEPTION("Timeout waiting for response to connect to MQ server");
    }

    m_socketGroup.add(*m_mqSocket, this);

    debug("CONNECTED");
}

void Client::_loadSslKeys(const string& keyFileName, const string& certificateFileName, const string& password,
                          const string& caFileName, int verifyMode, int verifyDepth)
{
    m_sslContext.loadKeys(keyFileName, certificateFileName, password, caFileName, verifyMode, verifyDepth);
}

void Client::_messageLoop(bool singleMessage)
{
    //static int counter = 0;
    uint32_t receiveTimeoutMS = 1000;

    mqtt::Frame frame(MT_UNDEFINED, 0, QOS_0);
    string topicName;
    while (!terminated()) {
        if (receiveFrame(frame, receiveTimeoutMS, topicName)) {
            switch (frame.type()) {
                case MT_CONNACK:
                case MT_SUBACK:
                case MT_UNSUBACK:
                case MT_PUBACK:
                    try {
                        uint16_t messageId = htons(*(uint16_t*)frame.c_str());
                        long ackMessageId = (frame.type() << 16) + messageId;
                        m_ackDispatcher.ack(ackMessageId);
                     }
                    catch(const exception& e) {
                        cerr << "ACK error: " << e.what() << endl;
                    }
                    break;

                case MT_PUBLISH:
                    {
                        uint16_t messageId = frame.id();
                        QOS      messageQOS = frame.qos();
                        MQMessage message(frame);
                        message.destination(topicName);
                        m_incomingMessages.push(move(message));
                        if (messageQOS == QOS_1) {
                            Frame ackFrame(MT_PUBACK, messageId, QOS_0);
                            ackFrame.publishAckFrame(messageId, MT_PUBACK);
                            send(ackFrame, false, 0);
                        }
                    }
                    break;

                case MT_PUBREC:
                    {
                        uint16_t messageId = frame.id();
                        Frame ackFrame(MT_PUBREL, messageId, QOS_0);
                        ackFrame.publishAckFrame(messageId, MT_PUBREL);
                        send(ackFrame, false, 0);
                    }
                    break;

                case MT_PUBREL:
                    {
                        uint16_t messageId = frame.id();
                        Frame ackFrame(MT_PUBCOMP, messageId, QOS_0);
                        ackFrame.publishAckFrame(messageId, MT_PUBCOMP);
                        send(ackFrame, false, 0);
                    }
                    break;

                default:
                    break;
            }

            if (singleMessage)
                break;

            continue;
        }
    }
}

void Client::_disconnect(bool polite)
{
    if (polite) {
        try {
            // Send DISCONNECT frame
            Frame frame(MT_DISCONNECT, 0, QOS_0);
            frame.append((char) 0);
            send(frame, false, 0);
        }
        catch (...) {}
    }

    {
        lock_guard<mutex> lock(m_mutex);

        // Terminate TCP connection
        m_readBuffer.reset();

        if (m_mqSocket != nullptr) {
            if (m_mqSocket->active())
                m_socketGroup.remove(*m_mqSocket);
            m_mqSocket->disconnect();
            delete m_mqSocket;
            m_mqSocket = nullptr;
        }
    }
    debug("DISCONNECTED");
}

// Receives MQTT frame
bool Client::receiveFrame(Frame& frame, uint32_t timeoutMS, string& topicName)
{
    bool rc;

    try {
        lock_guard<mutex>   lock(m_mutex);
        rc = frame.receive(*m_mqSocket, timeoutMS, topicName) != 0;
        debug("Received " + frame.typeName());
    }
    catch (...) {
        if (!m_mqSocket->active())
            rc = false;
        else
            throw;
    }

    setLastActive();
    return rc;
}

// Sends MQTT frame
void Client::send(const Frame& frame, bool waitForACK, uint32_t waitTimeoutMS)
{
    if (!m_mqSocket->active())
        THROW_EXCEPTION("Can't send frame to MQ server: not connected");

    if (frame.bytes() >= 64*1024)
        THROW_EXCEPTION("Can't send frame to MQ server: message is too large");

    long ackMessageId = 0;
    Semaphore* semaphore = nullptr;

    if (waitForACK) {
        MessageType _ackType = Frame::ackType(frame.type());
        if (_ackType != MT_UNDEFINED) {
            ackMessageId = (_ackType << 16) + frame.id();
            semaphore = m_ackDispatcher.willWaitForACK(ackMessageId);
        }
    }

    debug("Send " + frame.typeName());

    try {
        lock_guard<mutex>   lock(m_mutex);
        // Send MQTT frame
        m_mqSocket->write(frame.c_str(), frame.bytes());
    }
    catch (const exception& e) {
        cerr << "Can't send frame to MQ server: " << e.what() << endl;
        disconnect(false);
        delete semaphore;
        THROW_EXCEPTION("Can't send frame to MQ server, disconnecting");
    }

    if (semaphore != nullptr) {
        debug("Wait for ACK from server");

        bool rc = semaphore->wait(waitTimeoutMS);
        delete semaphore;
        if (!rc)
            THROW_TIMEOUT_EXCEPTION("Timeout waiting for ACK " << hex << (ackMessageId >> 16) << dec << " for message id " << (ackMessageId & 0xFFFF) );
    }

    if (frame.type() != MT_PINGREQ && frame.type() != MT_DISCONNECT)
        setLastActive();
}

void Client::subscribe(string destination, bool waitForACK, uint32_t waitTimeoutMS)
{
    // Create and send SUBSCRIBE frame
    QOS qos = waitForACK ? QOS_1 : QOS_0;
    Frame frame(MT_SUBSCRIBE, 0, qos);

    frame.subscribeFrame(destination, qos);

    send(frame, waitForACK, waitTimeoutMS);
}

void Client::subscribe(const Strings& destinations, bool waitForACK, uint32_t waitTimeoutMS)
{
    // Create and send SUBSCRIBE frame
    QOS qos = waitForACK ? QOS_1 : QOS_0;
    Frame frame(MT_SUBSCRIBE, 0, qos);

    frame.subscribeFrame(destinations, qos);

    send(frame, waitForACK, waitTimeoutMS);
}

void Client::unsubscribe(string destination, bool waitForACK, uint32_t waitTimeoutMS)
{
    // Create and send UNSUBSCRIBE frame
    QOS qos = waitForACK ? QOS_1 : QOS_0;
    Frame frame( MT_UNSUBSCRIBE, 0, qos);

    frame.unsubscribeFrame(destination, qos);

    send(frame, waitForACK, waitTimeoutMS);
}

void Client::unsubscribe(const Strings& destination, bool waitForACK, uint32_t waitTimeoutMS)
{
    // Create and send UNSUBSCRIBE frame
    QOS qos = waitForACK ? QOS_1 : QOS_0;
    Frame frame( MT_UNSUBSCRIBE, 0, qos);

    frame.unsubscribeFrame(destination, qos);

    send(frame, waitForACK, waitTimeoutMS);
}

void Client::_subscribeToQueue(const string& queueName, bool waitForACK, uint32_t waitTimeoutMS)
{
    subscribe("/queue/" + queueName, waitForACK, waitTimeoutMS);
}

void Client::_unsubscribeFromQueue(const string& queueName, bool waitForACK, uint32_t waitTimeoutMS)
{
    unsubscribe("/queue/" + queueName, waitForACK, waitTimeoutMS);
}

void Client::_subscribeToTopic(const string& topicName, bool waitForACK, uint32_t waitTimeoutMS)
{
    subscribe(topicName, waitForACK, waitTimeoutMS);
}

void Client::_unsubscribeFromTopic(const string& topicName, bool waitForACK, uint32_t waitTimeoutMS)
{
    unsubscribe(topicName, waitForACK, waitTimeoutMS);
}

void Client::sendMessage(string destination, const MQMessage& message, bool waitForACK, uint32_t waitTimeoutMS)
{
    // Create and send PUBLISH frame
    QOS qos = waitForACK ? QOS_1 : QOS_0;
    Frame frame( MT_PUBLISH, 0, qos);

    bool compress = message.header("Content-Encoding") == "gzip";
    frame.publishFrame(destination, message.data(compress), qos, false, false);
    send(frame, waitForACK, waitTimeoutMS);
}

void Client::sendToQueue(const string& queueName, const MQMessage& message, bool waitForACK, uint32_t waitTimeoutMS)
{
    sendMessage("/queue/" + queueName, message, waitForACK, waitTimeoutMS);
}

void Client::sendToTopic(string topicName, const MQMessage& message, bool waitForACK, uint32_t waitTimeoutMS)
{
    sendMessage(topicName, message, waitForACK, waitTimeoutMS);
}

void Client::ack(const string& messageId, const string& transactionId)
{
    //mqtt::Frame ackFrame("ACK");
    //send(ackFrame);
}

void Client::nack(const string& messageId, const string& transactionId)
{
    //mqtt::Frame nackFrame("NACK");
    //send(nackFrame);
}

void Client::subscribeToTopics(const Strings& topicNames, uint32_t waitTimeoutMS)
{
    subscribe(topicNames, true, waitTimeoutMS);
}

void Client::unsubscribeFromTopics(const Strings& topicNames, uint32_t waitTimeoutMS)
{
    unsubscribe(topicNames, true, waitTimeoutMS);
}

void Client::ping()
{
    Frame pingFrame(MT_PINGREQ, 0, QOS_0);
    pingFrame.pingFrame();
    send(pingFrame, false, 0);
}

bool Client::connected()
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!MQClient::m_connected)
            return false;

        int msSinceLastReceived = (DateTime::Now() - getLastActive()).count();
        if (msSinceLastReceived <= (pingIntervalMS * 2 + 10000))
            return true;
    }

    //disconnect(false);

    return false;
}
