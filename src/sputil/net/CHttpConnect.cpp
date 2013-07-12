/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CHttpConnect.cpp  -  description
                             -------------------
    begin                : July 19 2003
    copyright            : (C) 2003-2012 by Alexey Parshin. All rights reserved.
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

CHttpConnect::CHttpConnect() {
    m_port = 80;
    m_requestHeaders["Content-Type"] = "application/x-www-form-urlencoded";
    m_requestHeaders["Connection"] = "close";
}

CHttpConnect::~CHttpConnect() {
    close();
}

#define RSP_BLOCK_SIZE 16384

void CHttpConnect::getResponse() {
    m_readBuffer.reset();

    CStrings headers;
    CBuffer read_buffer(RSP_BLOCK_SIZE);

    int    bytes, contentLength = 0;
    string header;

    /// Reading HTTP headers
    for (;;) {
        readLine(header);
        if (header.empty())
            break;
        headers.push_back(header);
    }

    m_responseHeaders.clear();

    if (headers.empty())
        throw CException("Can't detect HTTP headers");

    /// Parsing HTTP headers
    if ( headers[0].find("HTTP/1.") != 0)
        throw CException("Broken HTTP version header");
    
    m_responseHeaders["HTTP version"] = headers[0];

    bool chunked = 0;
    for (unsigned i = 0; i < headers.size(); i++) {
        char* header = (char* ) headers[i].c_str();
        char* headerValue = strstr(header,": ");
        if (!headerValue)
            continue;
        *headerValue = 0;
        headerValue += 2;
        m_responseHeaders[header] = headerValue;
        if (strcmp(header,"Transfer-Encoding") == 0) {
            chunked = strcmp(headerValue,"chunked") == 0;
	    continue;
        }
        if (strcmp(header,"Content-Length") == 0) {
            contentLength = atoi(headerValue);
	    continue;
        }
    }

    if (!chunked) {
	int totalBytes = 0;
        for (;;) {
            bytes = (int) read(read_buffer,0);

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
            readLine(chunkSizeStr);
            if (chunkSizeStr.empty())
                readLine(chunkSizeStr);
            if (chunkSizeStr == "0")
                break;
            long chunkSize = strtol(chunkSizeStr.c_str(),0L,16);
            read_buffer.checkSize(chunkSize);
            bytes = (int) read(read_buffer.data(),chunkSize,0);
            if (bytes > 0) {
                read_buffer.data()[bytes] = 0;
                m_readBuffer.append(read_buffer);
            }
        }
    }
    close();
}

void CHttpConnect::sendCommand(string cmd) {
    cmd += "\r\n";
    if (!active())
        throw CException("Socket isn't open");
    write(cmd.c_str(),(uint32_t)cmd.length());
}

void CHttpConnect::cmd_get(string pageName,CHttpParams& postData) {
    m_readBuffer.checkSize(1024);

    CBuffer buffer;
    postData.encode(buffer);

    string parameters(buffer.data());

    string command = "GET http://" + m_host + "/" + pageName;
    if (parameters.length())
        command += "?" + parameters;
    command += " HTTP/1.0\r\n";
    //command += "Host: " + serverName + ":"+ string(m_port) + "\n";
    sendCommand(command);

    getResponse();
}

void CHttpConnect::cmd_post(string pageName,CHttpParams& postData) {
    CStrings headers;

    headers.push_back("POST " + pageName + " HTTP/1.1");
    headers.push_back("HOST: " + m_host);

    map<string,string>::iterator itor = m_requestHeaders.begin();
    map<string,string>::iterator iend = m_requestHeaders.end();
    for (; itor != iend; itor++)
        headers.push_back(itor->first + ": " + itor->second);

    CBuffer buffer;
    postData.encode(buffer);

    headers.push_back("Content-Length: " + int2string((uint32_t)buffer.bytes()));

    string command = headers.asString( "\r\n") + "\r\n\r\n";

    command += buffer.data();
    command += "\r\n";
    sendCommand(command);

    getResponse();
}
