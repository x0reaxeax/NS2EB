#include <stdfuncs.h>
#include <Screen.h>

BOOLEAN MemCmp(
    LPVOID lpBuffer1,
    LPVOID lpBuffer2,
    UINTN uSize
) {
    for (UINTN i = 0; i < uSize; i++) {
        if (((BYTE*) lpBuffer1)[i] != ((BYTE*) lpBuffer2)[i]) {
            return FALSE;
        }
    }
    return TRUE;
}

VOID MemSet(
    LPVOID lpDest,
    BYTE bValue,
    UINTN uSize
) {
    for (UINTN i = 0; i < uSize; i++) {
        ((BYTE*) lpDest)[i] = bValue;
    }
}

VOID MemCpy(
    LPVOID lpDest,
    LPVOID lpSrc,
    UINTN uSize
) {
    for (UINTN i = 0; i < uSize; i++) {
        ((BYTE*) lpDest)[i] = ((BYTE*) lpSrc)[i];
    }
}

LPBYTE MemSearch(
    LPVOID lpStart,
    UINTN uMaxSearchBytes,
    LPBYTE lpPattern,
    UINTN uPatternSize
) {
    LPBYTE lpEnd = (LPBYTE) ((UINT64) lpStart + uMaxSearchBytes);
    for (LPBYTE lpCurrent = (LPBYTE) lpStart; lpCurrent < lpEnd; lpCurrent++) {
        BOOLEAN bFound = TRUE;
        for (UINTN i = 0; i < uPatternSize; i++) {
            if (lpCurrent[i] != lpPattern[i]) {
                bFound = FALSE;
                break;
            }
        }
        if (bFound) {
            return lpCurrent;
        }
    }
    return NULL;
}

UINTN StrLen8(
    CHAR* String
) {
    UINTN Length = 0;
    while (String[Length] != '\0') {
        Length++;
    }
    return Length;
}

UINTN StrLen16(
    CHAR16* String
) {
    UINTN Length = 0;
    while (String[Length] != '\0') {
        Length++;
    }
    return Length;
}

VOID StrCpy8(
    CHAR* Dest,
    CONST CHAR* Src
) {
    UINTN i = 0;
    while (Src[i] != '\0') {
        Dest[i] = Src[i];
        i++;
    }
    Dest[i] = '\0';
}

VOID StrCpy16(
    CHAR16* Dest,
    CONST CHAR16* Src
) {
    UINTN i = 0;
    while (Src[i] != '\0') {
        Dest[i] = Src[i];
        i++;
    }
    Dest[i] = '\0';
}

VOID StrCat8(
    CHAR* Dest,
    CONST CHAR* Src
) {
    UINTN i = 0;
    while (Dest[i] != '\0') {
        i++;
    }
    StrCpy8(Dest + i, Src);
}

CHAR *StrStr8(
    CHAR *String,
    CHAR *SubString
) {
    UINTN StringLength = StrLen8(String);
    UINTN SubStringLength = StrLen8(SubString);
    for (UINTN i = 0; i < StringLength - SubStringLength; i++) {
        if (MemCmp(String + i, SubString, SubStringLength)) {
            return String + i;
        }
    }
    return NULL;
}

CHAR *StrnCat8(
    CHAR *Dest,
    CHAR *Src,
    UINTN uSize
) {
    UINTN DestLength = StrLen8(Dest);
    UINTN SrcLength = StrLen8(Src);
    UINTN i = 0;
    while (i < uSize && i < SrcLength) {
        Dest[DestLength + i] = Src[i];
        i++;
    }
    Dest[DestLength + i] = '\0';
    return Dest;
}

CHAR *StrChr8(
    CHAR *String,
    CHAR Char
) {
    UINTN StringLength = StrLen8(String);
    for (UINTN i = 0; i < StringLength; i++) {
        if (String[i] == Char) {
            return String + i;
        }
    }
    return NULL;
}


// Thanks ChatGPT <3
INT32 EFIAPI Vsprintf8(
    CHAR *Buffer, 
    CONST CHAR *Format, 
    VA_LIST Args
) {
    INT32 Length = 0;

    while (*Format != '\0') {
        if (*Format == '%') {
            Format++;
            int is_long = 0;
            int is_short = 0;
            int is_long_double = 0;
            int width = 0;
            int pad_zero = 0;

            // Handle width and padding
            if (*Format == '0') {
                pad_zero = 1;
                Format++;
            }
            while (*Format >= '0' && *Format <= '9') {
                width = width * 10 + (*Format - '0');
                Format++;
            }

            // Handle long and short modifiers
            if (*Format == 'l') {
                if (*(Format + 1) == 'f') {
                    is_long_double = 1;
                    Format++;
                } else {
                    is_long = 1;
                }
                Format++;
            } else if (*Format == 'h') {
                is_short = 1;
                Format++;
            }

            switch (*Format) {
                case 'd':
                case 'i': {
                    INT64 Value = is_long ? VA_ARG(Args, INT64) : (is_short ? (short)VA_ARG(Args, INT32) : VA_ARG(Args, INT32));
                    if (Value < 0) {
                        Buffer[Length++] = '-';
                        Value = -Value;
                    }
                    CHAR TempBuffer[32];
                    INT32 TempLength = 0;
                    do {
                        TempBuffer[TempLength++] = (Value % 10) + '0';
                        Value /= 10;
                    } while (Value > 0);
                    // Pad with zeros if necessary
                    while (TempLength < width) {
                        Buffer[Length++] = pad_zero ? '0' : ' ';
                        width--;
                    }
                    for (INT32 i = TempLength - 1; i >= 0; i--) {
                        Buffer[Length++] = TempBuffer[i];
                    }
                    break;
                }
                case 'u': {
                    UINT64 Value = is_long ? VA_ARG(Args, UINT64) : (is_short ? (unsigned short)VA_ARG(Args, UINT32) : VA_ARG(Args, UINT32));
                    CHAR TempBuffer[32];
                    INT32 TempLength = 0;
                    do {
                        TempBuffer[TempLength++] = (Value % 10) + '0';
                        Value /= 10;
                    } while (Value > 0);
                    // Pad with zeros if necessary
                    while (TempLength < width) {
                        Buffer[Length++] = pad_zero ? '0' : ' ';
                        width--;
                    }
                    for (INT32 i = TempLength - 1; i >= 0; i--) {
                        Buffer[Length++] = TempBuffer[i];
                    }
                    break;
                }
                case 'o': {
                    UINT64 Value = is_long ? VA_ARG(Args, UINT64) : VA_ARG(Args, UINT32);
                    CHAR TempBuffer[32];
                    INT32 TempLength = 0;
                    do {
                        TempBuffer[TempLength++] = (Value % 8) + '0';
                        Value /= 8;
                    } while (Value > 0);
                    // Pad with zeros if necessary
                    while (TempLength < width) {
                        Buffer[Length++] = pad_zero ? '0' : ' ';
                        width--;
                    }
                    for (INT32 i = TempLength - 1; i >= 0; i--) {
                        Buffer[Length++] = TempBuffer[i];
                    }
                    break;
                }
                case 'x':
                case 'X': {
                    UINT64 Value = is_long ? VA_ARG(Args, UINT64) : VA_ARG(Args, UINT32);
                    CHAR TempBuffer[32];
                    INT32 TempLength = 0;
                    do {
                        CHAR Digit = Value % 16;
                        TempBuffer[TempLength++] = (Digit < 10) ? (Digit + '0') : (Digit - 10 + (*Format == 'x' ? 'a' : 'A'));
                        Value /= 16;
                    } while (Value > 0);
                    // Pad with zeros if necessary
                    while (TempLength < width) {
                        Buffer[Length++] = pad_zero ? '0' : ' ';
                        width--;
                    }
                    for (INT32 i = TempLength - 1; i >= 0; i--) {
                        Buffer[Length++] = TempBuffer[i];
                    }
                    break;
                }
                case 'p': {
                    QWORD Value = (QWORD)VA_ARG(Args, void*);
                    Buffer[Length++] = '0';
                    Buffer[Length++] = 'x';
                    CHAR TempBuffer[32];
                    INT32 TempLength = 0;
                    do {
                        CHAR Digit = Value % 16;
                        TempBuffer[TempLength++] = (Digit < 10) ? (Digit + '0') : (Digit - 10 + 'a');
                        Value /= 16;
                    } while (Value > 0);
                    // Pad with zeros if necessary
                    while (TempLength < width) {
                        Buffer[Length++] = '0';
                        width--;
                    }
                    for (INT32 i = TempLength - 1; i >= 0; i--) {
                        Buffer[Length++] = TempBuffer[i];
                    }
                    break;
                }
                case 'f':
                case 'e':
                case 'E': {
                    double Value = is_long_double ? VA_ARG(Args, long double) : VA_ARG(Args, double);
                    if (Value < 0) {
                        Buffer[Length++] = '-';
                        Value = -Value;
                    }
                    INT32 IntPart = (INT32)Value;
                    double FracPart = Value - IntPart;
                    CHAR TempBuffer[32];
                    INT32 TempLength = 0;
                    do {
                        TempBuffer[TempLength++] = (IntPart % 10) + '0';
                        IntPart /= 10;
                    } while (IntPart > 0);
                    // Pad with zeros if necessary
                    while (TempLength < width) {
                        Buffer[Length++] = pad_zero ? '0' : ' ';
                        width--;
                    }
                    for (INT32 i = TempLength - 1; i >= 0; i--) {
                        Buffer[Length++] = TempBuffer[i];
                    }
                    Buffer[Length++] = '.';
                    for (INT32 i = 0; i < 6; i++) {
                        FracPart *= 10;
                        Buffer[Length++] = (INT32)FracPart + '0';
                        FracPart -= (INT32)FracPart;
                    }
                    break;
                }
                case 'L': { // Handle %Lf
                    if (*(Format + 1) == 'f') {
                        Format++; // Skip 'f'
                        long double Value = VA_ARG(Args, long double);
                        if (Value < 0) {
                            Buffer[Length++] = '-';
                            Value = -Value;
                        }
                        INT64 IntPart = (INT64)Value;
                        long double FracPart = Value - IntPart;
                        CHAR TempBuffer[64];
                        INT32 TempLength = 0;
                        do {
                            TempBuffer[TempLength++] = (IntPart % 10) + '0';
                            IntPart /= 10;
                        } while (IntPart > 0);
                        // Pad with zeros if necessary
                        while (TempLength < width) {
                            Buffer[Length++] = pad_zero ? '0' : ' ';
                            width--;
                        }
                        for (INT32 i = TempLength - 1; i >= 0; i--) {
                            Buffer[Length++] = TempBuffer[i];
                        }
                        Buffer[Length++] = '.';
                        for (INT32 i = 0; i < 10; i++) { // Increase precision
                            FracPart *= 10;
                            Buffer[Length++] = (INT32)FracPart + '0';
                            FracPart -= (INT32)FracPart;
                        }
                    }
                    break;
                }
                case 'c': {
                    CHAR Value = (CHAR)VA_ARG(Args, INT32);
                    Buffer[Length++] = Value;
                    break;
                }
                case 's': {
                    CHAR *Value = VA_ARG(Args, CHAR*);
                    while (*Value != '\0') {
                        Buffer[Length++] = *Value++;
                    }
                    break;
                }
                case '%': {
                    Buffer[Length++] = '%';
                    break;
                }
                case 'n': {
                    *(VA_ARG(Args, INT32*)) = Length;
                    break;
                }
                default: {
                    Buffer[Length++] = '%';
                    Buffer[Length++] = *Format;
                    break;
                }
            }
        } else {
            Buffer[Length++] = *Format;
        }
        Format++;
    }
    Buffer[Length] = '\0';
    return Length;
}

INT32 EFIAPI Sprintf8(
    CHAR *Buffer, 
    const CHAR *Format,
    ...
) {
    VA_LIST Args;
    VA_START(Args, Format);
    INT32 Result = Vsprintf8(Buffer, Format, Args);
    VA_END(Args);
    return Result;
}

CHAR8 *Itoa8(
    UINTN Value,
    CHAR8 *Buffer,
    UINTN Base
) {
    CHAR8 *p = Buffer;
    CHAR8 *p1, *p2;
    UINTN n = Value;
    CHAR8 c;
    UINTN r;

    if (Base < 2 || Base > 36) {
        *p = 0;
        return p;
    }

    if (n < 0) {
        n = -n;
        *p++ = '-';
    }

    p1 = p;

    do {
        r = n % Base;
        *p++ = (CHAR8)(r < 10 ? r + '0' : r + 'A' - 10);
        n /= Base;
    } while (n > 0);

    p2 = p;
    *p-- = 0;

    while (p1 < p) {
        c = *p;
        *p-- = *p1;
        *p1++ = c;
    }

    return p2;
}

CHAR16 *Itoa16(
    UINTN Value,
    CHAR16 *Buffer,
    UINTN Base
) {
    CHAR16 *p = Buffer;
    CHAR16 *p1, *p2;
    UINTN n = Value;
    CHAR16 c;
    UINTN r;

    if (Base < 2 || Base > 36) {
        *p = 0;
        return p;
    }

    if (n < 0) {
        n = -n;
        *p++ = '-';
    }

    p1 = p;

    do {
        r = n % Base;
        *p++ = (CHAR16)(r < 10 ? r + '0' : r + 'A' - 10);
        n /= Base;
    } while (n > 0);

    p2 = p;
    *p-- = 0;

    while (p1 < p) {
        c = *p;
        *p-- = *p1;
        *p1++ = c;
    }

    return p2;
}

UINTN WcsToMbs(
    CHAR *Dest,
    CHAR16 *Src,
    UINTN uSize
) {
    UINTN i = 0;
    while (Src[i] != '\0' && i < uSize) {
        Dest[i] = (CHAR) Src[i];
        i++;
    }
    Dest[i] = '\0';
    return i;
}

UINTN MbsToWcs(
    CHAR16 *Dest,
    CHAR *Src,
    UINTN uSize
) {
    UINTN i = 0;
    while (Src[i] != '\0' && i < uSize) {
        Dest[i] = (CHAR16) Src[i];
        i++;
    }
    Dest[i] = '\0';
    return i;
}

EFI_STATUS FWrite(
    LPVOID Buffer,
    UINTN Size
) {
    EFI_STATUS Status;
    Status = g_LogFile->Write(
        g_LogFile,
        &Size,
        Buffer
    );
    return Status;
}