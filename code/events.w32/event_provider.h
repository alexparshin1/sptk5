 // Event categories mirror the syslog facility codes, so that the same SysLogEngine
 // facility groups messages the same way on Windows and on *nix:
 // category N is syslog facility N-1.
//
//  Values are 32 bit values laid out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +---+-+-+-----------------------+-------------------------------+
//  |Sev|C|R|     Facility          |               Code            |
//  +---+-+-+-----------------------+-------------------------------+
//
//  where
//
//      Sev - is the severity code
//
//          00 - Success
//          01 - Informational
//          10 - Warning
//          11 - Error
//
//      C - is the Customer code flag
//
//      R - is a reserved bit
//
//      Facility - is the facility code
//
//      Code - is the facility's status code
//
//
// Define the facility codes
//
#define FACILITY_SYSTEM                  0x0
#define FACILITY_RUNTIME                 0x2
#define FACILITY_STUBS                   0x3
#define FACILITY_IO_ERROR_CODE           0x4


//
// Define the severity codes
//
#define STATUS_SEVERITY_SUCCESS          0x0
#define STATUS_SEVERITY_INFORMATIONAL    0x1
#define STATUS_SEVERITY_WARNING          0x2
#define STATUS_SEVERITY_ERROR            0x3


//
// MessageId: SPTK_CATEGORY_KERN
//
// MessageText:
//
// Kernel
//
#define SPTK_CATEGORY_KERN               ((WORD)0x00000001L)

//
// MessageId: SPTK_CATEGORY_USER
//
// MessageText:
//
// User
//
#define SPTK_CATEGORY_USER               ((WORD)0x00000002L)

//
// MessageId: SPTK_CATEGORY_MAIL
//
// MessageText:
//
// Mail
//
#define SPTK_CATEGORY_MAIL               ((WORD)0x00000003L)

//
// MessageId: SPTK_CATEGORY_DAEMON
//
// MessageText:
//
// Daemon
//
#define SPTK_CATEGORY_DAEMON             ((WORD)0x00000004L)

//
// MessageId: SPTK_CATEGORY_AUTH
//
// MessageText:
//
// Security/Authorization
//
#define SPTK_CATEGORY_AUTH               ((WORD)0x00000005L)

//
// MessageId: SPTK_CATEGORY_SYSLOG
//
// MessageText:
//
// Syslog
//
#define SPTK_CATEGORY_SYSLOG             ((WORD)0x00000006L)

//
// MessageId: SPTK_CATEGORY_LPR
//
// MessageText:
//
// Line Printer
//
#define SPTK_CATEGORY_LPR                ((WORD)0x00000007L)

//
// MessageId: SPTK_CATEGORY_NEWS
//
// MessageText:
//
// Network News
//
#define SPTK_CATEGORY_NEWS               ((WORD)0x00000008L)

//
// MessageId: SPTK_CATEGORY_UUCP
//
// MessageText:
//
// UUCP
//
#define SPTK_CATEGORY_UUCP               ((WORD)0x00000009L)

//
// MessageId: SPTK_CATEGORY_CRON
//
// MessageText:
//
// Clock Daemon
//
#define SPTK_CATEGORY_CRON               ((WORD)0x0000000AL)

//
// MessageId: SPTK_CATEGORY_AUTHPRIV
//
// MessageText:
//
// Security/Authorization (private)
//
#define SPTK_CATEGORY_AUTHPRIV           ((WORD)0x0000000BL)

//
// MessageId: SPTK_CATEGORY_FTP
//
// MessageText:
//
// FTP Daemon
//
#define SPTK_CATEGORY_FTP                ((WORD)0x0000000CL)

//
// MessageId: SPTK_CATEGORY_RESERVED12
//
// MessageText:
//
// Reserved 12
//
#define SPTK_CATEGORY_RESERVED12         ((WORD)0x0000000DL)

//
// MessageId: SPTK_CATEGORY_RESERVED13
//
// MessageText:
//
// Reserved 13
//
#define SPTK_CATEGORY_RESERVED13         ((WORD)0x0000000EL)

//
// MessageId: SPTK_CATEGORY_RESERVED14
//
// MessageText:
//
// Reserved 14
//
#define SPTK_CATEGORY_RESERVED14         ((WORD)0x0000000FL)

//
// MessageId: SPTK_CATEGORY_RESERVED15
//
// MessageText:
//
// Reserved 15
//
#define SPTK_CATEGORY_RESERVED15         ((WORD)0x00000010L)

//
// MessageId: SPTK_CATEGORY_LOCAL0
//
// MessageText:
//
// Local 0
//
#define SPTK_CATEGORY_LOCAL0             ((WORD)0x00000011L)

//
// MessageId: SPTK_CATEGORY_LOCAL1
//
// MessageText:
//
// Local 1
//
#define SPTK_CATEGORY_LOCAL1             ((WORD)0x00000012L)

//
// MessageId: SPTK_CATEGORY_LOCAL2
//
// MessageText:
//
// Local 2
//
#define SPTK_CATEGORY_LOCAL2             ((WORD)0x00000013L)

//
// MessageId: SPTK_CATEGORY_LOCAL3
//
// MessageText:
//
// Local 3
//
#define SPTK_CATEGORY_LOCAL3             ((WORD)0x00000014L)

//
// MessageId: SPTK_CATEGORY_LOCAL4
//
// MessageText:
//
// Local 4
//
#define SPTK_CATEGORY_LOCAL4             ((WORD)0x00000015L)

//
// MessageId: SPTK_CATEGORY_LOCAL5
//
// MessageText:
//
// Local 5
//
#define SPTK_CATEGORY_LOCAL5             ((WORD)0x00000016L)

//
// MessageId: SPTK_CATEGORY_LOCAL6
//
// MessageText:
//
// Local 6
//
#define SPTK_CATEGORY_LOCAL6             ((WORD)0x00000017L)

//
// MessageId: SPTK_CATEGORY_LOCAL7
//
// MessageText:
//
// Local 7
//
#define SPTK_CATEGORY_LOCAL7             ((WORD)0x00000018L)

 // The following are the message definitions.
//
// MessageId: MSG_TEXT
//
// MessageText:
//
// %1
//
#define MSG_TEXT                         ((DWORD)0xC0020100L)

//
// MessageId: MSG_BAD_FILE_CONTENTS
//
// MessageText:
//
// File %1 contains content that is not valid.
//
#define MSG_BAD_FILE_CONTENTS            ((DWORD)0xC0000101L)

//
// MessageId: MSG_RETRIES
//
// MessageText:
//
// There have been %1 retries with %2 success! Disconnect from
// the server and try again later.
//
#define MSG_RETRIES                      ((DWORD)0x80000102L)

//
// MessageId: MSG_COMPUTE_CONVERSION
//
// MessageText:
//
// %1 %%4096 = %2 %%4097. 
//
#define MSG_COMPUTE_CONVERSION           ((DWORD)0x40000103L)

 // The following are the parameter strings */
//
// MessageId: QUARTS_UNITS
//
// MessageText:
//
// quarts%0
//
#define QUARTS_UNITS                     ((DWORD)0x00001000L)

//
// MessageId: GALLONS_UNITS
//
// MessageText:
//
// gallons%0
//
#define GALLONS_UNITS                    ((DWORD)0x00001001L)

