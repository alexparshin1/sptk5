/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       WSProtocol.cpp - description                           ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Saturday Jul 30 2016                                   ║
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

#include <sptk5/wsdl/WSProtocol.h>

using namespace std;
using namespace sptk;

WSStaticHttpProtocol::WSStaticHttpProtocol(TCPSocket *socket, String url, const std::map<String,String>& headers, String staticFilesDirectory)
: WSProtocol(socket, headers), m_url(url), m_staticFilesDirectory(staticFilesDirectory)
{
    
}

void WSStaticHttpProtocol::process()
{
    Buffer page;
    try {
        page.loadFromFile(m_staticFilesDirectory + m_url);
        m_socket.write("HTTP/1.1 200 OK\n");
        m_socket.write("Content-Type: text/html; charset=utf-8\n");
        m_socket.write("Content-Length: " + int2string(page.bytes()) + "\n\n");
        m_socket.write(page);
    }
    catch (...) {
        string text("<html><head><title>Not Found</title></head><body>Sorry, the page you requested was not found.</body></html>\n");
        m_socket.write("HTTP/1.1 404 Not Found\n");
        m_socket.write("Content-Type: text/html; charset=utf-8\n");
        m_socket.write("Content-length: " + int2string(text.length()) + "\n\n");
        m_socket.write(text);
    }
}

WSWebSocketsProtocol::WSWebSocketsProtocol(TCPSocket *socket, const std::map<String,String>& headers)
: WSProtocol(socket, headers)
{

}

void WSWebSocketsProtocol::process()
{
}

WSWebServiceProtocol::WSWebServiceProtocol(TCPSocket *socket, const std::map<String,String>& headers, WSRequest& service)
: WSProtocol(socket, headers), m_service(service)
{
    
}

void WSWebServiceProtocol::process()
{
    int contentLength = 0;
    map<String,String>::const_iterator itor = m_headers.find("Content-Length");
    if (itor != m_headers.end())
        contentLength = string2int(itor->second);
    
    m_socket.write("<?xml version='1.0' encoding='UTF-8'?><server name='" + m_service.title() + "' version='1.0'/>\n");
    uint32_t offset = 0;

    const char* startOfMessage = NULL;
    const char* endOfMessage = NULL;
    const char* endOfMessageMark = ":Envelope>";

    Buffer data;
    
    if (contentLength) {
        m_socket.read(data, contentLength);
        startOfMessage = data.c_str();
        endOfMessage = startOfMessage + data.bytes();
    } else {
        uint32_t socketBytes = m_socket.socketBytes();
        if (!socketBytes) {
            if (!m_socket.readyToRead(30000))
                throwException("Client disconnected");
            socketBytes = m_socket.socketBytes();
        }
        
        // If socket is signaled but empty - then other side closed connection
        if (socketBytes == 0)
            throwException("Client disconnected");

        do {
            // Read all available data (appending to data buffer)
            data.checkSize(offset + socketBytes);
            socketBytes = (uint32_t) m_socket.read(data.data() + offset, (uint32_t) socketBytes);
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
        } while (!endOfMessage);

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

    m_socket.write(output);
}
