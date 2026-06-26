/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin. All rights reserved.       ║
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

#include <sptk5/net/LogDetails.h>

using namespace std;
using namespace sptk;

const map<String, LogDetails::MessageDetail> LogDetails::detailNames {
    {"serial_id", MessageDetail::SERIAL_ID},
    {"source_ip", MessageDetail::SOURCE_IP},
    {"request_name", MessageDetail::REQUEST_NAME},
    {"request_duration", MessageDetail::REQUEST_DURATION},
    {"request_data", MessageDetail::REQUEST_DATA},
    {"response_data", MessageDetail::RESPONSE_DATA},
    {"thread_pooling", MessageDetail::THREAD_POOLING}};

LogDetails::LogDetails(const Strings& details)
{
    for (const auto& detailName: details)
    {
        const auto itor = detailNames.find(detailName);
        if (itor == detailNames.end())
        {
            continue;
        }
        m_details.insert(itor->second);
    }
}

String LogDetails::toString(const String& delimiter) const
{
    Strings names;
    for (const auto& [name, value]: detailNames)
    {
        if (m_details.contains(value))
        {
            names.push_back(name);
        }
    }
    return names.join(delimiter.c_str());
}
