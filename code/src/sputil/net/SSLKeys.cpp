/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2024 Alexey Parshin. All rights reserved.       ║
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

#include "sptk5/net/SSLKeys.h"
#include <sptk5/Buffer.h>

using namespace std;
using namespace sptk;

SSLKeys::SSLKeys(filesystem::path privateKeyFileName, filesystem::path certificateFileName,
                 String password, filesystem::path caFileName, int verifyMode,
                 int verifyDepth)
    : m_privateKeyFileName(move(privateKeyFileName))
    , m_certificateFileName(move(certificateFileName))
    , m_password(move(password))
    , m_caFileName(move(caFileName))
    , m_verifyMode(verifyMode)
    , m_verifyDepth(verifyDepth)
{
}

SSLKeys::SSLKeys(const SSLKeys& other)
{
    const scoped_lock lock(m_mutex);
    assign(other);
}

SSLKeys& SSLKeys::operator=(const SSLKeys& other)
{
    const scoped_lock lock(m_mutex, other.m_mutex);
    if (&other == this)
    {
        return *this;
    }
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

filesystem::path SSLKeys::privateKeyFileName() const
{
    const scoped_lock lock(m_mutex);
    return m_privateKeyFileName;
}

filesystem::path SSLKeys::certificateFileName() const
{
    const scoped_lock lock(m_mutex);
    return m_certificateFileName;
}

String SSLKeys::password() const
{
    const scoped_lock lock(m_mutex);
    return m_password;
}

filesystem::path SSLKeys::caFileName() const
{
    const scoped_lock lock(m_mutex);
    return m_caFileName;
}

int SSLKeys::verifyMode() const
{
    const scoped_lock lock(m_mutex);
    return m_verifyMode;
}

int SSLKeys::verifyDepth() const
{
    const scoped_lock lock(m_mutex);
    return m_verifyDepth;
}

String SSLKeys::ident() const
{
    Buffer buffer;
    buffer.append(m_privateKeyFileName);
    buffer.append('~');
    buffer.append(m_certificateFileName);
    buffer.append('~');
    buffer.append(m_caFileName);
    buffer.append('~');
    buffer.append(to_string(m_verifyMode));
    buffer.append('~');
    buffer.append(to_string(m_verifyDepth));
    return {buffer.c_str(), buffer.size()};
}

bool SSLKeys::empty() const
{
    return m_certificateFileName.empty();
}
