#include "Screen.h"
#include "stdfuncs.h"

#define MAX_SAFE_SCREEN_WIDTH  80

GLOBAL UINTN g_ScreenWidth = 0;
GLOBAL UINTN g_ScreenHeight = 0;

GLOBAL CHAR16 g_LogBuffer[
    OUTPUT_LOGLINE_END - OUTPUT_LOGLINE_START
][MAX_SAFE_SCREEN_WIDTH] = { 0 };

GLOBAL VOID *g_TempMemory1 = NULL;
GLOBAL VOID *g_TempMemory2 = NULL;

VOID UpdateLogLines(
    CHAR16 *NewLine
) {
    for (UINTN i = 0; i < OUTPUT_LOGLINE_END - 1; i++) {
        StrCpy(g_LogBuffer[i], g_LogBuffer[i + 1]);
    }
    StrCpy(g_LogBuffer[OUTPUT_LOGLINE_END - 1], NewLine);
}

VOID ClearLine(
    UINTN Line
) {
    for (UINTN i = 0; i < g_ScreenWidth; i++) {
        PrintAt(i, Line, L" ");
    }
}

VOID ClearLinesFromTo(
    UINTN Start,
    UINTN End
) {
    for (UINTN i = Start; i < End; i++) {
        ClearLine(i);
    }
}

VOID PrintLog(
    VOID
) {
    ClearLinesFromTo(OUTPUT_LOGLINE_START, OUTPUT_LOGLINE_END);

    for (UINTN i = 0; i < ARRAYSIZE(g_LogBuffer); i++) {
        PrintAt(0, OUTPUT_LOGLINE_START + i, g_LogBuffer[i]);
    }
}

VOID AddLogLine(
    BOOLEAN DisplaToScreen,
    BOOLEAN LogToFile,
    CHAR16 *Line,
    ...
) {
    va_list Args;
    va_start(Args, Line);

    MemSet(g_TempMemory1, 0, 0x1000);
    MemSet(g_TempMemory2, 0, 0x1000);
    VSPrint(g_TempMemory1, 0x1000-1, Line, Args);

    if (DisplaToScreen) {
        UpdateLogLines(g_TempMemory1);
        PrintLog();
    }

    if (LogToFile) {

        UINTN LogLen = StrLen(g_TempMemory1);
        WcsToMbs(
            g_TempMemory2,
            g_TempMemory1,
            LogLen
        );

        FWrite(
            g_TempMemory2,
            LogLen
        );
    }

    va_end(Args);
}

VOID DisplayAverageMSRTime(
    UINT64 Time,
    BOOLEAN bValidMSR
) {
    CONST UINTN OutputLine = bValidMSR ? OUTPUT_AVERAGE_GOOD_TIME : OUTPUT_AVERAGE_BAD_TIME;
    ClearLine(OutputLine);
    CHAR16 TextBuffer[64] = { 0 };
    SPrint(
        TextBuffer, 
        64, 
        L"* %s MSR read time: %llu", 
        bValidMSR ? L"Valid" : L"Invalid",
        Time
    );

    PrintAt(
        0,
        OutputLine, 
        L"[*] Average %s MSR read time: %llu", 
        bValidMSR ? L"Valid" : L"Invalid",
        Time
    );
}

INT32 InitializeScreen(
    VOID
) {
    EFI_STATUS Status;

    Status = uefi_call_wrapper(
        ST->ConOut->ClearScreen,
        1,
        ST->ConOut
    );

    if (EFI_ERROR(Status)) {
        Print(L"[-] Failed to clear screen: E%lx\n", Status);
        return Status;
    }

    Status = uefi_call_wrapper(
        ST->ConOut->QueryMode,
        4,
        ST->ConOut,
        ST->ConOut->Mode->Mode,
        &g_ScreenWidth,
        &g_ScreenHeight
    );

    if (EFI_ERROR(Status)) {
        Print(L"[-] Failed to get screen dimensions: E%lx\n", Status);
        return Status;
    }

    AddLogLineScreen(L"[*] Screen dimensions: %llu x %llu\n", g_ScreenWidth, g_ScreenHeight);

    return EFI_SUCCESS;
}