SeverityNames=(Success=0x0:STATUS_SEVERITY_SUCCESS
               Informational=0x1:STATUS_SEVERITY_INFORMATIONAL
               Warning=0x2:STATUS_SEVERITY_WARNING
               Error=0x3:STATUS_SEVERITY_ERROR
              )


FacilityNames=(System=0x0:FACILITY_SYSTEM
               Runtime=0x2:FACILITY_RUNTIME
               Stubs=0x3:FACILITY_STUBS
               Io=0x4:FACILITY_IO_ERROR_CODE
              )

LanguageNames=(English=0x409:MSG00409)


; // Event categories mirror the syslog facility codes, so that the same SysLogEngine
; // facility groups messages the same way on Windows and on *nix:
; // category N is syslog facility N-1.

MessageIdTypedef=WORD

MessageId=0x1
SymbolicName=SPTK_CATEGORY_KERN
Language=English
Kernel
.

MessageId=0x2
SymbolicName=SPTK_CATEGORY_USER
Language=English
User
.

MessageId=0x3
SymbolicName=SPTK_CATEGORY_MAIL
Language=English
Mail
.

MessageId=0x4
SymbolicName=SPTK_CATEGORY_DAEMON
Language=English
Daemon
.

MessageId=0x5
SymbolicName=SPTK_CATEGORY_AUTH
Language=English
Security/Authorization
.

MessageId=0x6
SymbolicName=SPTK_CATEGORY_SYSLOG
Language=English
Syslog
.

MessageId=0x7
SymbolicName=SPTK_CATEGORY_LPR
Language=English
Line Printer
.

MessageId=0x8
SymbolicName=SPTK_CATEGORY_NEWS
Language=English
Network News
.

MessageId=0x9
SymbolicName=SPTK_CATEGORY_UUCP
Language=English
UUCP
.

MessageId=0xa
SymbolicName=SPTK_CATEGORY_CRON
Language=English
Clock Daemon
.

MessageId=0xb
SymbolicName=SPTK_CATEGORY_AUTHPRIV
Language=English
Security/Authorization (private)
.

MessageId=0xc
SymbolicName=SPTK_CATEGORY_FTP
Language=English
FTP Daemon
.

MessageId=0xd
SymbolicName=SPTK_CATEGORY_RESERVED12
Language=English
Reserved 12
.

MessageId=0xe
SymbolicName=SPTK_CATEGORY_RESERVED13
Language=English
Reserved 13
.

MessageId=0xf
SymbolicName=SPTK_CATEGORY_RESERVED14
Language=English
Reserved 14
.

MessageId=0x10
SymbolicName=SPTK_CATEGORY_RESERVED15
Language=English
Reserved 15
.

MessageId=0x11
SymbolicName=SPTK_CATEGORY_LOCAL0
Language=English
Local 0
.

MessageId=0x12
SymbolicName=SPTK_CATEGORY_LOCAL1
Language=English
Local 1
.

MessageId=0x13
SymbolicName=SPTK_CATEGORY_LOCAL2
Language=English
Local 2
.

MessageId=0x14
SymbolicName=SPTK_CATEGORY_LOCAL3
Language=English
Local 3
.

MessageId=0x15
SymbolicName=SPTK_CATEGORY_LOCAL4
Language=English
Local 4
.

MessageId=0x16
SymbolicName=SPTK_CATEGORY_LOCAL5
Language=English
Local 5
.

MessageId=0x17
SymbolicName=SPTK_CATEGORY_LOCAL6
Language=English
Local 6
.

MessageId=0x18
SymbolicName=SPTK_CATEGORY_LOCAL7
Language=English
Local 7
.


; // The following are the message definitions.

MessageIdTypedef=DWORD

MessageId=0x100
Severity=Error
Facility=Runtime
SymbolicName=MSG_TEXT
Language=English
%1
.


MessageId=0x101
Severity=Error
Facility=System
SymbolicName=MSG_BAD_FILE_CONTENTS
Language=English
File %1 contains content that is not valid.
.

MessageId=0x102
Severity=Warning
Facility=System
SymbolicName=MSG_RETRIES
Language=English
There have been %1 retries with %2 success! Disconnect from
the server and try again later.
.

MessageId=0x103
Severity=Informational
Facility=System
SymbolicName=MSG_COMPUTE_CONVERSION
Language=English
%1 %%4096 = %2 %%4097. 
.


; // The following are the parameter strings */


MessageId=0x1000
Severity=Success
Facility=System
SymbolicName=QUARTS_UNITS
Language=English
quarts%0
.

MessageId=0x1001
Severity=Success
Facility=System
SymbolicName=GALLONS_UNITS
Language=English
gallons%0
.

