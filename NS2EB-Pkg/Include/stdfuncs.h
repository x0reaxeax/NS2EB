#ifndef _STD_FUNCS_H
#define _STD_FUNCS_H

#include "x86intrin.h"

#define ARRAYSIZE(x) (sizeof(x) / sizeof(x[0]))

BOOLEAN MemCmp(
    LPVOID lpBuffer1,
    LPVOID lpBuffer2,
    UINTN uSize
);

VOID MemSet(
    LPVOID lpDest,
    BYTE bValue,
    UINTN uSize
);

VOID MemCpy(
    LPVOID lpDest,
    LPVOID lpSrc,
    UINTN uSize
);

LPBYTE MemSearch(
    LPVOID lpStart,
    UINTN uMaxSearchBytes,
    LPBYTE lpPattern,
    UINTN uPatternSize
);

UINTN StrLen8(
    CHAR* String
);

UINTN StrLen16(
    CHAR16* String
);

VOID StrCpy8(
    CHAR* Dest,
    CONST CHAR* Src
);

VOID StrCat8(
    CHAR* Dest,
    CONST CHAR* Src
);

CHAR *StrStr8(
    CHAR *String,
    CHAR *SubString
);

CHAR *StrnCat8(
    CHAR *Dest,
    CHAR *Src,
    UINTN uSize
);

CHAR *StrChr8(
    CHAR *String,
    CHAR Char
);

INT32 Sprintf8(
    CHAR *Buffer,
    CONST CHAR *Format,
    ...
);

INT32 Vsprintf8(
    CHAR *Buffer, 
    CONST CHAR *Format, 
    va_list Args
);

CHAR8 *Itoa8(
    UINTN Value,
    CHAR8 *Buffer,
    UINTN Base
);

CHAR16 *Itoa16(
    UINTN Value,
    CHAR16 *Buffer,
    UINTN Base
);

UINTN WcsToMbs(
    CHAR *Dest,
    CHAR16 *Src,
    UINTN uSize
);

UINTN MbsToWcs(
    CHAR16 *Dest,
    CHAR *Src,
    UINTN uSize
);

EFI_STATUS FWrite(
    LPVOID Buffer,
    UINTN Size
);

#endif