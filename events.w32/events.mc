MessageIdTypedef=DWORD

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


; // Categories of events:

MessageIdTypedef=WORD

MessageId=0x1
SymbolicName=SPTK_MSG_CATEGORY
Language=English
SPTK events
.


; // Message definitions:

MessageIdTypedef=WORD

MessageId=0x1
Severity=Error
Facility=Runtime
SymbolicName=SPTK_MSG
Language=English
%1
.

;