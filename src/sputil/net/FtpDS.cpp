/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       FtpDS.cpp - description                                ║
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

#include <sptk5/net/CFtpDS.h>
#include <sptk5/CSmallPixmapIDs.h>

#ifndef FL_ALIGN_RIGHT
#define FL_ALIGN_RIGHT 8
#endif

using namespace std;
using namespace sptk;

static char* next_dir_item(char* p, char** result)
{
    char* start = p;
    for (; *start == ' '; start++);
    *result = start;
    start = strchr(start, ' ');
    *start = 0;
    return start + 1;
}

static const Strings
    month_names("Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Nov|Dec", "|");

FieldList* parse_file_info_string(string& file_info)
{
    auto ptr = (char*) file_info.c_str();

    char* permissions = nullptr;
    char* refcount = nullptr;
    char* user_name = nullptr;
    char* group_name = nullptr;
    char* size = nullptr;
    char* time = nullptr;
    char* file_name = nullptr;
    bool is_directory = false;
    bool is_executable = false;

    DateTime dt;
    CSmallPixmapType pixmapType = SXPM_DOCUMENT;

    if (isdigit(*ptr) != 0) {
        // MS Dos style
        char* dtime = nullptr;

        ptr = next_dir_item(ptr, &time);
        ptr = next_dir_item(ptr, &dtime);

        ptr = next_dir_item(ptr, &size);

        if (strstr(size, "DIR") != nullptr)
            is_directory = true;
        time[2] = 0;
        time[5] = 0;

        auto month = (int) strtol(time, nullptr, 10);
        auto day = (int) strtol(time + 3, nullptr, 10);
        auto year = (int) strtol(time + 6, nullptr, 10);

        bool pm = false;
        if (strstr(dtime, "PM") != nullptr)
            pm = true;
        dtime[2] = 0;
        dtime[5] = 0;
        auto hour = (int) strtol(dtime, nullptr, 10);
        auto min = (int) strtol(dtime + 3, nullptr, 10);
        if (pm) hour += 12;

        if (year < 50) year += 2000;
        else year += 1900;

        DateTime dosDate((short) year, (short) month, (short) day, (short) hour, (short) min);
        dt = dosDate;
    } else {
        char* month = nullptr, * day = nullptr, * year = nullptr;

        ptr = next_dir_item(ptr, &permissions);
        // Unix style
        if (permissions[0] == 'd')
            is_directory = true;
        else if (strchr(permissions, 'x') != nullptr)
            is_executable = true;
        ptr = next_dir_item(ptr, &refcount);
        ptr = next_dir_item(ptr, &user_name);
        ptr = next_dir_item(ptr, &group_name);
        ptr = next_dir_item(ptr, &size);
        ptr = next_dir_item(ptr, &month);
        ptr = next_dir_item(ptr, &day);
        ptr = next_dir_item(ptr, &year);
        int m = month_names.indexOf(month) + 1;
        if (m >= 0) {
            int d = atoi(day);
            int y = atoi(year);
            DateTime unixDate((short) y, (short) m, (short) d);
            dt = unixDate;
        }
    }

    if (is_directory)
        pixmapType = SXPM_FOLDER;
    else if (is_executable)
        pixmapType = SXPM_EXECUTABLE;

    // skip to the file name
    for (; *ptr == ' '; ptr++);
    file_name = ptr;

    auto df = new FieldList(false);

    df->push_back("", false).setImageNdx(pixmapType);
    df->push_back("Name", false) = file_name;
    df->push_back("Size", false) = (uint32_t) atoi(size);
    df->push_back("Modified", false) = dt;

    (*df)[uint32_t(0)].view.width = 3;
    (*df)[uint32_t(1)].view.width = 30;
    (*df)[uint32_t(2)].view.width = 10;
    (*df)[uint32_t(2)].view.flags = FL_ALIGN_RIGHT;
    (*df)[uint32_t(3)].view.width = 16;

    return df;
}

// read the folder() and move item into the first entry
bool sptk::FtpDS::open() THROWS_EXCEPTIONS
{
    clear();

    // Connect to the server
    m_ftp.host(m_host, m_port);
    m_ftp.user(m_user);
    m_ftp.password(m_password);
    m_ftp.open();

    // Select the folder
    if (!m_folder.empty())
        m_ftp.cmd_cd(m_folder);

    Strings dirlist;
    m_ftp.cmd_list(dirlist);
    //dirlist.print();

    if (!dirlist.empty()) {
        auto cnt = (int) dirlist.size();
        if (m_callback != nullptr)
            m_callback(cnt, 0);
        for (int i = 0; i < cnt; i++) {
            FieldList* df = parse_file_info_string(dirlist[size_t(i)]);
            if (df != nullptr)
                m_list.push_back(df);

            if (m_callback != nullptr)
                m_callback(cnt, i);
        }
    }

    if (m_callback != nullptr)
        m_callback(100, 100);

    first();

    m_ftp.cmd_quit();
    m_ftp.close();

    m_eof = m_list.empty();

    return !m_eof;
}
