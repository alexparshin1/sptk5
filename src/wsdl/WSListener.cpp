/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CWSListener.cpp - description                          ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2016 by Alexey Parshin. All rights reserved.  ║
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

#include <sptk5/wsdl/WSListener.h>

using namespace std;
using namespace sptk;

class CWSConnection : public TCPServerConnection
{
protected:
    WSRequest&     m_service;
    Logger&       m_logger;
    const string&   m_staticFilesDirectory;
public:

    CWSConnection(SOCKET connectionSocket, sockaddr_in*, WSRequest& service, Logger& logger, const string& staticFilesDirectory)
    : TCPServerConnection(connectionSocket), m_service(service), m_logger(logger), m_staticFilesDirectory(staticFilesDirectory)
    {
    }

    virtual ~CWSConnection()
    {
    }

    virtual void threadFunction();
};

void CWSConnection::threadFunction()
{
    static const RegularExpression parseProtocol("^(GET|POST) (\\S+)", "i");
    static const RegularExpression parseHeader("^([^:]+): \"{0,1}(.*)\"{0,1}$", "i");

    const char* startOfMessage = NULL;
    const char* endOfMessage = NULL;
    const char* endOfMessageMark = ":Envelope>";

    Buffer data;

    // Read request data
    string      row;
    Strings     matches;
    string      protocol, url, requestType;
    int         contentLength = 0;

    try {
        if (!m_socket->readyToRead(30000)) {
            m_socket->close();
            m_logger << LP_DEBUG << "Client closed connection" << endl;
            return;
        }

        try {
            while (!terminated()) {
                if (!m_socket->readLine(data))
                    return;
                row = trim(data.c_str());
                if (protocol.empty()) {
                    if (strstr(row.c_str(), "<?xml")) {
                        protocol = "xml";
                        break;
                    }
                    if (parseProtocol.m(row, matches)) {
                        protocol = "http";
                        requestType = matches[0];
                        url = matches[1];
                        continue;
                    }
                }
                if (parseHeader.m(row, matches)) {
                    string header = matches[0];
                    string value = matches[1];
                    if (lowerCase(header) == "content-length")
                        contentLength = string2int(value);
                    continue;
                }
                if (row.empty()) {
                    data.reset();
                    break;
                }
            }
        }
        catch (Exception& e) {
            m_logger << LP_ERROR << e.message() << endl;
            return;
        }
        catch (exception& e) {
            m_logger << LP_ERROR << e.what() << endl;
            return;
        }

        if (protocol == "http" && !url.empty() && url != "/service.html") {
            Buffer page;
            try {
                page.loadFromFile(m_staticFilesDirectory + url);
                m_socket->write("HTTP/1.1 200 OK\n");
                m_socket->write("Content-Type: text/html; charset=utf-8\n");
                m_socket->write("Content-Length: " + int2string(page.bytes()) + "\n\n");
                m_socket->write(page);
            }
            catch (...) {
                string text("<html><head><title>Not Found</title></head><body>Sorry, the page you requested was not found.</body></html>\n");
                m_socket->write("HTTP/1.1 404 Not Found\n");
                m_socket->write("Content-Type: text/html; charset=utf-8\n");
                m_socket->write("Content-length: " + int2string(text.length()) + "\n\n");
                m_socket->write(text);
            }
            m_socket->close();
            return;
        }

        if (protocol == "xml")
            m_socket->write("<?xml version='1.0' encoding='UTF-8'?><server name='" + m_service.title() + "' version='1.0'/>\n");
        uint32_t offset = 0;
        while (!terminated()) {
            if (contentLength) {
                m_socket->read(data, contentLength);
                startOfMessage = data.c_str();
                endOfMessage = startOfMessage + data.bytes();
            } else {
                uint32_t socketBytes = m_socket->socketBytes();
                if (!socketBytes) {
                    if (!m_socket->readyToRead(30000)) {
                        m_logger <<"Client disconnected" << endl;
                        break;
                    }
                    socketBytes = m_socket->socketBytes();
                }
                // If socket is signaled but empty - then other side closed connection
                if (socketBytes == 0) {
                    m_logger <<"Client disconnected" << endl;
                    break;
                }
                do {
                    // Read all available data (appending to data buffer)
                    data.checkSize(offset + socketBytes);
                    socketBytes = (uint32_t) m_socket->read(data.data() + offset, (uint32_t) socketBytes);
                    data.bytes(offset + socketBytes);
                    //cout << data.c_str() << endl;
                    if (!startOfMessage) {
                        startOfMessage = strstr(data.c_str(), "<?xml");
                        if (!startOfMessage) {
                            startOfMessage = strstr(data.c_str(), "Envelope");
                            if (startOfMessage)
                                while (*startOfMessage != '<' && startOfMessage > data.c_str())
                                    startOfMessage--;
                        }
                        if (!startOfMessage)
                            throwException("Message start <?xml> not found");
                    }
                    endOfMessage = strstr(startOfMessage, endOfMessageMark);
                } while (!endOfMessage && !terminated());

                if (terminated())
                    break;

                // Message received, processing it
                endOfMessage += strlen(endOfMessageMark);
            }

            sptk::XMLDocument message;
            if (endOfMessage)
                *(char *) endOfMessage = 0;
            message.load(startOfMessage);

            //cout << startOfMessage << endl << endl;

            Buffer output;
            m_service.processRequest(&message);
            message.save(output);

            //cout << output.c_str() << endl;

            if (protocol == "http") {
                m_socket->write("HTTP/1.1 200 OK\n");
                m_socket->write("Content-Type: text/xml; charset=utf-8\n");
                m_socket->write("Content-Length: " + int2string(output.bytes()) + "\n\n");
            }
            m_socket->write(output);
            m_socket->close();
            break;
        }
    }
    catch (exception& e) {
        if (!terminated())
            m_logger << LP_ERROR << "Error in thread " << name() << ": " << e.what() << endl;
    }
    catch (...) {
        if (!terminated())
            m_logger << LP_ERROR << "Unknown error in thread " << name() << endl;
    }
}

WSListener::WSListener(WSRequest& service, Logger& logger, string staticFilesDirectory)
: TCPServer(), m_service(service), m_logger(logger), m_staticFilesDirectory(staticFilesDirectory)
{
}

WSListener::~WSListener()
{
}

ServerConnection* WSListener::createConnection(SOCKET connectionSocket, sockaddr_in* peer)
{
    return new CWSConnection(connectionSocket, peer, m_service, m_logger, m_staticFilesDirectory);
}
