/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       SSLKeys.cpp - description                              ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Friday Feb 8 2019                                      ║
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

#include <sptk5/Buffer.h>

#include <utility>
#include "sptk5/net/SSLKeys.h"

using namespace std;
using namespace sptk;

SSLKeys::SSLKeys(String privateKeyFileName, String certificateFileName,
                 String password, String caFileName, int verifyMode,
                 int verifyDepth)
: m_privateKeyFileName(std::move(privateKeyFileName)), m_certificateFileName(std::move(certificateFileName)),
  m_password(std::move(password)), m_caFileName(std::move(caFileName)), m_verifyMode(verifyMode), m_verifyDepth(verifyDepth)
{
}

SSLKeys::SSLKeys(const SSLKeys& other)
{
    SharedLock(other.m_mutex);
    assign(other);
}

SSLKeys& SSLKeys::operator=(const SSLKeys& other)
{
    CopyLock(m_mutex, other.m_mutex);
    if (&other == this)
        return *this;
    assign(other);
    return *this;
}

void SSLKeys::assign(const SSLKeys& other)
{
    m_privateKeyFileName = other.m_privateKeyFileName;
    m_certificateFileName = other.m_certificateFileName;
    m_password = other.m_password;
    m_caFileName = other.m_caFileName;
    m_verifyMode = other.m_verifyMode;
    m_verifyDepth = other.m_verifyDepth;
}

String SSLKeys::privateKeyFileName() const
{
    SharedLock(m_mutex);
    return m_privateKeyFileName;
}

String SSLKeys::certificateFileName() const
{
    SharedLock(m_mutex);
    return m_certificateFileName;
}

String SSLKeys::password() const
{
    SharedLock(m_mutex);
    return m_password;
}

String SSLKeys::caFileName() const
{
    SharedLock(m_mutex);
    return m_caFileName;
}

int SSLKeys::verifyMode() const
{
    SharedLock(m_mutex);
    return m_verifyMode;
}

int SSLKeys::verifyDepth() const
{
    SharedLock(m_mutex);
    return m_verifyDepth;
}

String SSLKeys::ident() const
{
    Buffer buffer;
    buffer.append(m_privateKeyFileName); buffer.append('~');
    buffer.append(m_certificateFileName); buffer.append('~');
    buffer.append(m_caFileName); buffer.append('~');
    buffer.append(to_string(m_verifyMode)); buffer.append('~');
    buffer.append(to_string(m_verifyDepth));
    return String(buffer.c_str(), buffer.length());
}
