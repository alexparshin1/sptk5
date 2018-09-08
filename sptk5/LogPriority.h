#ifndef __LOG_PRIORITY_H__
#define __LOG_PRIORITY_H__

#ifndef _WIN32

#include <syslog.h>

#else
/* priority codes */
#define LOG_EMERG   0   /* system is unusable */
#define LOG_ALERT   1   /* action must be taken immediately */
#define LOG_CRIT    2   /* critical conditions */
#define LOG_ERR     3   /* error conditions */
#define LOG_WARNING 4   /* warning conditions */
#define LOG_NOTICE  5   /* normal but significant condition */
#define LOG_INFO    6   /* informational */
#define LOG_DEBUG   7   /* debug-level messages */

    /* facility codes */
#define LOG_KERN        (0<<3)  /* kernel messages */
#define LOG_USER        (1<<3)  /* random user-level messages */
#define LOG_MAIL        (2<<3)  /* mail system */
#define LOG_DAEMON      (3<<3)  /* system daemons */
#define LOG_AUTH        (4<<3)  /* security/authorization messages */
#define LOG_SYSLOG      (5<<3)  /* messages generated internally by syslogd */
#define LOG_LPR         (6<<3)  /* line printer subsystem */
#define LOG_NEWS        (7<<3)  /* network news subsystem */
#define LOG_UUCP        (8<<3)  /* UUCP subsystem */
#define LOG_CRON        (9<<3)  /* clock daemon */
#endif

namespace sptk {

/**
 * @addtogroup log Log Classes
 * @{
 */

/**
 * @brief Log message priority
 */
enum LogPriority : unsigned
{
    /**
     * Debug message priority
     */
    LP_DEBUG = LOG_DEBUG,

    /**
     * Information message priority
     */
    LP_INFO = LOG_INFO,

    /**
     * Notice message priority
     */
    LP_NOTICE = LOG_NOTICE,

    /**
     * Warning message priority
     */
    LP_WARNING = LOG_WARNING,

    /**
     * Error message priority
     */
    LP_ERROR = LOG_ERR,

    /**
     * Critical message priority
     */
    LP_CRITICAL = LOG_CRIT,

    /**
     * Alert message priority
     */
    LP_ALERT = LOG_ALERT,

    /**
     * Panic message priority
     */
    LP_PANIC = LOG_EMERG
};

/**
 * @}
 */
}

#endif
