/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2022 Alexey Parshin. All rights reserved.       ║
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

#pragma once

#ifndef _WIN32

#include <syslog.h>

#else
/* priority codes */
#define LOG_EMERG 0         /* system is unusable */
#define LOG_ALERT 1         /* action must be taken immediately */
#define LOG_CRIT 2          /* critical conditions */
#define LOG_ERR 3           /* error conditions */
#define LOG_WARNING 4       /* warning conditions */
#define LOG_NOTICE 5        /* normal but significant condition */
#define LOG_INFO 6          /* informational */
#define LOG_DEBUG 7         /* debug-level messages */

/* facility codes */
#define LOG_KERN (0 << 3)   /* kernel messages */
#define LOG_USER (1 << 3)   /* random user-level messages */
#define LOG_MAIL (2 << 3)   /* mail system */
#define LOG_DAEMON (3 << 3) /* system daemons */
#define LOG_AUTH (4 << 3)   /* security/authorization messages */
#define LOG_SYSLOG (5 << 3) /* messages generated internally by syslogd */
#define LOG_LPR (6 << 3)    /* line printer subsystem */
#define LOG_NEWS (7 << 3)   /* network news subsystem */
#define LOG_UUCP (8 << 3)   /* UUCP subsystem */
#define LOG_CRON (9 << 3)   /* clock daemon */
#endif

namespace sptk {

/**
 * @addtogroup log Log Classes
 * @{
 */

/**
 * @brief Log message priority
 */
enum class LogPriority
{
    /**
     * Debug message priority
     */
    DEBUG = LOG_DEBUG,

    /**
     * Information message priority
     */
    INFO = LOG_INFO,

    /**
     * Notice message priority
     */
    NOTICE = LOG_NOTICE,

    /**
     * Warning message priority
     */
    WARNING = LOG_WARNING,

    /**
     * Error message priority
     */
    ERR = LOG_ERR,

    /**
     * Critical message priority
     */
    CRITICAL = LOG_CRIT,

    /**
     * Alert message priority
     */
    ALERT = LOG_ALERT,

    /**
     * Panic message priority
     */
    PANIC = LOG_EMERG
};

/**
 * @}
 */
} // namespace sptk
