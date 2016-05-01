/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CFTPConnect.cpp  -  description
                             -------------------
    begin                : July 19 2003
    copyright            : (C) 1999-2016 by Alexey Parshin. All rights reserved.
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

#ifndef _WIN32
#    include <netinet/in.h>
#endif

#include <sptk5/net/CFTPConnect.h>
#include <stdio.h>
#include <string.h>

using namespace sptk;

CFTPSocket::CFTPSocket()
: CTCPSocket()
{
    m_port = 21;
    m_type = SOCK_STREAM;
    m_protocol = IPPROTO_TCP;
}

CFTPSocket::~CFTPSocket()
{
    if (active())
        write("QUIT\n", 6);
}

void CFTPSocket::open(std::string hostName, uint32_t port, CSocketOpenMode openMode) THROWS_EXCEPTIONS
{
    CTCPSocket::open(hostName, port, openMode);
    get_response();
    int on = 1;
    setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEADDR, (char *) &on, sizeof (on));
}

const CStrings& CFTPSocket::login(std::string user, std::string password)
{
    command("USER " + user);
    return command("PASS " + password);
}

const CStrings& CFTPSocket::get_response()
{
    char readBuffer[255];
    char retCode[5];

    m_response.clear();

    // read the first line of response
    size_t bytes = readLine(readBuffer, 255);
    m_response.push_back(std::string(readBuffer, bytes));

    // read the return code
    if (readBuffer[3] == '-') {
        readBuffer[3] = ' ';
        readBuffer[4] = 0;
        strcpy(retCode, readBuffer);
        for (;;) {
            bytes = readLine(readBuffer, 255);
            m_response.push_back(std::string(readBuffer, bytes));
            readBuffer[4] = 0;
            if (strcmp(readBuffer, retCode) == 0)
                break;
        }
    }
    return m_response;
}

const CStrings& CFTPSocket::command(std::string cmd)
{
    write((cmd + "\n").c_str(), (uint32_t) cmd.length() + 1);
    return get_response();
}

CFTPConnect::CFTPConnect()
: m_commandSocket(), m_dataSocket()
{
    m_passive = true;
}

CFTPConnect::~CFTPConnect()
{
    close();
}

void CFTPConnect::host(std::string hostName, uint32_t portNumber)
{
    close();
    m_port = portNumber;
    m_host = hostName;
}

void CFTPConnect::open()
{
    m_commandSocket.open(m_host, m_port);
    m_commandSocket.login(m_user, m_password);
}

void CFTPConnect::close()
{
    if (!active())
        return;
    m_commandSocket.close();
    m_dataSocket.close();
}

void CFTPConnect::command(std::string cmd)
{
    if (!active())
        throw CException("Connection doesn't exist yet");
    m_commandSocket.command(cmd);
}

void CFTPConnect::openDataPort()
{

    union
    {
        struct sockaddr sa;
        struct sockaddr_in in;
    } sin;
    uint32_t l = sizeof (sin);
    uint32_t v[6];
    struct linger lng = {
        0, 0
    };

    if (m_passive) {
        command("PASV");
        const std::string& resp = response()[0];
        if (resp[0] != '2')
            throw CException(resp);

        memset(&sin, 0, l);
        sin.in.sin_family = AF_INET;
        const char *cp = strchr(resp.c_str(), '(');
        if (cp == NULL)
            throw CException(resp);
        cp++;
        if (sscanf(cp, "%u,%u,%u,%u,%u,%u", &v[2], &v[3], &v[4], &v[5], &v[0], &v[1]) < 6)
            throw CException("Can't understand server response");
        for (unsigned i = 0; i < 6; i++)
            sin.sa.sa_data[i] = (char) v[i];
    }
    // Current implementation supports only passive mode, sorry
    //if (m_passive) {
    m_dataSocket.open_addr(CTCPSocket::SOM_CONNECT, &sin.in);
    setsockopt(m_dataSocket.handle(), SOL_SOCKET, SO_LINGER, (char *) &lng, sizeof (lng));
    //}
}

void CFTPConnect::cmd_quit()
{
    command("QUIT");
    close();
}

void CFTPConnect::cmd_type(char type)
{
    std::string mode("TYPE I");
    mode[5] = type;
    command(mode);
}

void CFTPConnect::cmd_cd(std::string dir)
{
    command("CWD " + dir);
}

void CFTPConnect::cmd_pwd()
{
    command("PWD ");
}

void CFTPConnect::getList(std::string cmd, CStrings& list)
{
    CBuffer buffer(1024);
    openDataPort();
    command(cmd);
    size_t len;
    list.clear();
    do {
        len = m_dataSocket.readLine(buffer);
        if (len)
            list.push_back(std::string(buffer.data(), len));
    }
    while (len);
    m_dataSocket.close();
    m_commandSocket.get_response();
}

void CFTPConnect::cmd_list(CStrings& result)
{
    getList("LIST", result);
}

void CFTPConnect::cmd_nlst(CStrings& result)
{
    getList("NLST", result);
}

void CFTPConnect::cmd_retr(std::string fileName)
{
    char *buffer = new char[2048];
    FILE *outfile = fopen(fileName.c_str(), "w+b");
    if (!outfile)
        throw CException("Can't open file <" + fileName + "> for writing");
    openDataPort();
    command("RETR " + fileName);
    size_t len;
    do {
        len = m_dataSocket.read(buffer, 2048, 0);
        if (len) {
            uint32_t bytes = (uint32_t) fwrite(buffer, 1, len, outfile);
            if (bytes != len) {
                delete buffer;
                throw CException("Can't open file <" + fileName + "> for writing");
            }
        }
    }
    while (len);
    m_dataSocket.close();
    fclose(outfile);
    m_commandSocket.get_response();
    delete [] buffer;
}

void CFTPConnect::cmd_store(std::string fileName)
{
    CBuffer buffer(8192);
    FILE *infile = fopen(fileName.c_str(), "rb");
    if (!infile)
        throw CException("Can't open file <" + fileName + "> for reading");
    openDataPort();
    command("STOR " + fileName);
    size_t len, bytes;
    while (!feof(infile)) {
        bytes = (uint32_t) fread(buffer.data(), 1, 8192, infile);
        char *p = buffer.data();
        while (bytes) {
            len = m_dataSocket.write(p, bytes);
            if (len == 0) {
                fclose(infile);
                m_dataSocket.close();
                throw CException("Can't send file <" + fileName + "> - transfer interrupted");
            }
            p += len;
            bytes -= len;
        }
        fflush(stdout);
    }
    m_dataSocket.write(NULL, 0);
    m_dataSocket.close();
    fclose(infile);
}
