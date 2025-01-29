#include <Screen.h>
#include <stdfuncs.h>

#define MAX_SAFE_SCREEN_WIDTH  80

GLOBAL EFI_FILE_PROTOCOL *g_LogFile = NULL;

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
        StrCpy16(g_LogBuffer[i], g_LogBuffer[i + 1]);
    }
    StrCpy16(g_LogBuffer[OUTPUT_LOGLINE_END - 1], NewLine);
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

VOID EFIAPI PrintAt(
    UINTN X, 
    UINTN Y, 
    CHAR16 *Format, 
    ...
) {
    VA_LIST Marker;
    VA_START(Marker, Format);

    // Move cursor
    gST->ConOut->SetCursorPosition(
        gST->ConOut, 
        X, 
        Y
    );

    // Buffer for formatted output
    CHAR16 Buffer[256] = { 0 };
    UnicodeVSPrint(Buffer, sizeof(Buffer), Format, Marker);

    // Print formatted output
    gST->ConOut->OutputString(
        gST->ConOut, 
        Buffer
    );

    VA_END(Marker);
};

VOID EFIAPI AddLogLine(
    BOOLEAN DisplaToScreen,
    BOOLEAN LogToFile,
    CHAR16 *Line,
    ...
) {
    VA_LIST Args;
    VA_START(Args, Line);

    MemSet(g_TempMemory1, 0, 0x1000);
    MemSet(g_TempMemory2, 0, 0x1000);
    UnicodeVSPrint(g_TempMemory1, 0x1000-1, Line, Args);

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

    VA_END(Args);
}

VOID EFIAPI DisplayAverageMSRTime(
    UINT64 Time,
    BOOLEAN bValidMSR
) {
    CONST UINTN OutputLine = bValidMSR ? OUTPUT_AVERAGE_GOOD_TIME : OUTPUT_AVERAGE_BAD_TIME;
    ClearLine(OutputLine);

    PrintAt(
        0,
        OutputLine,
        L"[*] Average %ls MSR read time: %llu", 
        bValidMSR ? L"Valid" : L"Invalid",
        Time
    );
}

INT32 InitializeScreen(
    VOID
) {
    EFI_STATUS Status;

    Status = gST->ConOut->ClearScreen(
        gST->ConOut
    );

    if (EFI_ERROR(Status)) {
        Print(L"[-] Failed to clear screen: E%lx\n", Status);
        return Status;
    }

    Status = gST->ConOut->QueryMode(
        gST->ConOut,
        gST->ConOut->Mode->Mode,
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