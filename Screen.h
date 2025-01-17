#ifndef _MSRFUZZ_SCREEN_H
#define _MSRFUZZ_SCREEN_H

#include "x86intrin.h"

EXTERN GLOBAL UINTN g_ScreenWidth;
EXTERN GLOBAL UINTN g_ScreenHeight;

EXTERN GLOBAL VOID *g_TempMemory1;
EXTERN GLOBAL VOID *g_TempMemory2;

EXTERN EFI_FILE_PROTOCOL *g_LogFile;

#define LOGFILE_NAME                L"ns2eb.log"

#define OUTPUT_LOGLINE_START        0
#define OUTPUT_LOGLINE_END          8

#define OUTPUT_CURRENT_MSR          g_ScreenHeight - 3
#define OUTPUT_AVERAGE_BAD_TIME     g_ScreenHeight - 4
#define OUTPUT_AVERAGE_GOOD_TIME    g_ScreenHeight - 5
#define OUTPUT_EXECFLAG_ADDR        g_ScreenHeight - 6

INT32 InitializeScreen(
    VOID
);

VOID DisplayAverageMSRTime(
    UINT64 Time,
    BOOLEAN bValidMSR
);

VOID AddLogLine(
    BOOLEAN DisplaToScreen,
    BOOLEAN LogToFile,
    CHAR16 *Line,
    ...
);

VOID ClearLine(
    UINTN Line
);

#define AddLogLineScreen(Line, ...) AddLogLine(TRUE, FALSE, Line __VA_OPT__(, __VA_ARGS__))
#define AddLogLineFile(Line, ...) AddLogLine(FALSE, TRUE, Line __VA_OPT__(, __VA_ARGS__))
#define AddLogLineVerbose(Line, ...) AddLogLine(TRUE, TRUE, Line __VA_OPT__(, __VA_ARGS__))


#endif // _MSRFUZZ_SCREEN_H