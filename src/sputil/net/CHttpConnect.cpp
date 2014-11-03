/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CHttpConnect.cpp  -  description
                             -------------------
    begin                : July 19 2003
    copyright            : (C) 1999-2014 by Alexey Parshin. All rights reserved.
    email                : alexeyp@gmail.com
 ***************************************************************************/

/***************************************************************************
   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at
   your option) any later version.

   This library is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library
   General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.

   Please report all bugs and problems to "alexeyp@gmail.com"
 ***************************************************************************/

#include <sptk5/sptk.h>
#include <sptk5/net/CHttpConnect.h>
#include <sptk5/net/CHttpParams.h>

#include <iostream>

using namespace std;
using namespace sptk;

CHttpConnect::CHttpConnect(CTCPSocket& socket)
: m_socket(socket)
{
    m_requestHeaders["Content-Type"] = "application/x-www-form-urlencoded";
    m_requestHeaders["Connection"] = "close";
}

CHttpConnect::~CHttpConnect()
{
    m_socket.close();
}

#define RSP_BLOCK_SIZE 1024*64

void CHttpConnect::getResponse()
{
    CBuffer read_buffer(RSP_BLOCK_SIZE);
    
    m_readBuffer.reset();

    CStrings headers;

    int    bytes, contentLength = 0;
    string header;

    /// Reading HTTP headers
    m_socket.readyToRead(1000);

    for (;;) {
        m_socket.readLine(header);
        char* tail = (char*) strpbrk(header.c_str(),"\r\n");

        if (tail)
            *tail = 0;

        if (tail == header.c_str())
            break;

        headers.push_back(header);
    }

    m_responseHeaders.clear();

    if (headers.empty())
        throw CException("Can't detect HTTP headers");

    /// Parsing HTTP headers
    if (headers[0].find("HTTP/1.") != 0)
        throw CException("Broken HTTP version header");

    m_responseHeaders["HTTP version"] = headers[0];

    bool chunked = 0;

    for (unsigned i = 0; i < headers.size(); i++) {
        char* headerStr = (char*) headers[i].c_str();
        char* headerValue = strstr(headerStr,": ");

        if (!headerValue)
            continue;

        *headerValue = 0;
        headerValue += 2;
        m_responseHeaders[headerStr] = headerValue;

        if (strcmp(headerStr,"Transfer-Encoding") == 0) {
            chunked = strstr(headerValue,"chunked") != NULL;
            continue;
        } else if (strcmp(headerStr,"Content-Length") == 0) {
            contentLength = atoi(headerValue);
            continue;
        }
    }

    if (!chunked) {
        int totalBytes = 0;

        for (;;) {
            bytes = (int) m_socket.read(read_buffer, 64*1024);

            if (bytes <= 0) // No more data
                break;

            m_readBuffer.append(read_buffer);
            totalBytes += bytes;

            if (contentLength && totalBytes >= contentLength)
                break;
        }
    } else {
        string chunkSizeStr;

        for (;;) {
            m_socket.readLine(chunkSizeStr);
            chunkSizeStr = trim(chunkSizeStr);

            if (chunkSizeStr.empty())
                m_socket.readLine(chunkSizeStr);

            size_t chunkSize = (size_t) strtol(chunkSizeStr.c_str(),0L,16);

            if (chunkSize == 0)
                break;

            read_buffer.checkSize(chunkSize);
            bytes = (int) m_socket.read(read_buffer.data(),chunkSize,0);

            if (bytes > 0) {
                read_buffer.data() [bytes] = 0;
                m_readBuffer.append(read_buffer);
            }
        }
    }

    m_socket.close();
}

void CHttpConnect::sendCommand(string cmd)
{
    cmd += "\n";

    if (!m_socket.active())
        throw CException("Socket isn't open");

    m_socket.write(cmd.c_str(), (uint32_t) cmd.length());
}

void CHttpConnect::cmd_get(string pageName,CHttpParams& postData)
{
    m_readBuffer.checkSize(1024);

    CBuffer buffer;
    postData.encode(buffer);

    string parameters(buffer.data());

    string command = "GET " + pageName;

    if (parameters.length())
        command += "?" + parameters;

    command += " HTTP/1.1\n";
    command += "User-Agent: Wget/1.15 (linux-gnu)\n";
    command += "Accept: */*\n";
    command += "Host: " + m_socket.host() + ":"+ int2string(m_socket.port()) + "\n";
    command += "Connection: Keep-Alive\n";

    CBuffer buff;
    buff.append(command);
    //buff.saveToFile("/tmp/http.cmd");

    sendCommand(command);

    getResponse();
}

void CHttpConnect::cmd_post(string pageName,CHttpParams& postData)
{
    CStrings headers;

    headers.push_back("POST " + pageName + " HTTP/1.1");
    headers.push_back("HOST: " + m_socket.host());

    map<string,string>::iterator itor = m_requestHeaders.begin();
    map<string,string>::iterator iend = m_requestHeaders.end();

    for (; itor != iend; itor++)
        headers.push_back(itor->first + ": " + itor->second);

    CBuffer buffer;
    postData.encode(buffer);

    headers.push_back("Content-Length: " + int2string((uint32_t) buffer.bytes()));

    string command = headers.asString("\r\n") + "\r\n\r\n";

    command += buffer.data();
    command += "\r\n";
    sendCommand(command);

    getResponse();
}
