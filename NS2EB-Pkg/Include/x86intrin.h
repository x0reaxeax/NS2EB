#ifndef _X86INTRIN_H
#define _X86INTRIN_H

#if !defined(GNU_EFI_USE_MS_ABI) || !defined (MS_ABI)
#define GNU_EFI_USE_MS_ABI 1
#define MS_ABI __attribute__((ms_abi))
#endif

#define SYSV_ABI __attribute__((sysv_abi))

#include <efi/efi.h>
#include <efi/efilib.h>

#define NAKED           __attribute__((naked))
#define UNUSED          __attribute_maybe_unused__
#define PURE            __attribute_pure__
#define MAYBE_UNUSED    UNUSED
#define PACKED          __attribute__((packed))
#define INLINE          __attribute__((always_inline))
#define EXTERN          extern
//#define MALLOC        __attribute_malloc__

#define GLOBAL

#define ANYSIZE_ARRAY 1

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif

typedef enum _READ_CONTEXT_REG {
    CTX_REG_RAX = 0,
    CTX_REG_RBX,
    CTX_REG_RCX,
    CTX_REG_RDX,
    CTX_REG_RSI,
    CTX_REG_RDI,
    CTX_REG_RBP,
    CTX_REG_R8,
    CTX_REG_R9,
    CTX_REG_R10,
    CTX_REG_R11,
    CTX_REG_R12,
    CTX_REG_R13,
    CTX_REG_R14,
    CTX_REG_R15
} READ_CONTEXT_REG;

#define __PLACE_BYTE(byte, count) \
    __asm__ __volatile__ ( \
        ".rept %c0\n.byte %c1\n.endr" \
        : \
        : "i" (count), "i" (byte) \
    )

typedef char            CHAR;
typedef CHAR8           BYTE;
typedef BYTE*           LPBYTE;
typedef VOID*           LPVOID;

typedef CHAR*           PCHAR;
typedef PCHAR           LPSTR;
typedef CHAR16*         LPWSTR;
typedef CONST LPSTR     LPCSTR;
typedef CONST LPWSTR    LPCWSTR;

typedef struct {
    UINT64 Present : 1;
    UINT64 ReadWrite : 1;
    UINT64 UserSupervisor : 1;
    UINT64 PageWriteThrough : 1;
    UINT64 PageCacheDisable : 1;
    UINT64 Accessed : 1;
    UINT64 Dirty : 1;
    UINT64 PageSize : 1;
    UINT64 Global : 1;
    UINT64 Available1 : 3;
    UINT64 PageFrameNumber : 40;
    UINT64 Available2 : 11;
    UINT64 NoExecute : 1;
} PageTableEntry;

typedef union {
    struct {
        UINT32  PE         : 1; // Protection Enable
        UINT32  MP         : 1; // Monitor coProcessor
        UINT32  EM         : 1; // Emulation
        UINT32  TS         : 1; // Task Switched
        UINT32  ET         : 1; // Extension Type
        UINT32  NE         : 1; // Numeric Error
        UINT32  Reserved0  : 10;
        UINT32  WP         : 1; // Write Protect
        UINT32  Reserved1  : 1;
        UINT32  AM         : 1; // Alignment Mask
        UINT32  Reserved2  : 10;
        UINT32  NW         : 1; // Not Write-through
        UINT32  CD         : 1; // Cache Disable
        UINT32  PG         : 1; // Paging
    } Bits;
    UINTN UintN;
} IA32_CR0, CR0_REGISTER;

// cr4
typedef union {
    struct {
        UINT32  VME        : 1; // Virtual-8086 Mode Extensions
        UINT32  PVI        : 1; // Protected-Mode Virtual Interrupts
        UINT32  TSD        : 1; // Time Stamp Disable
        UINT32  DE         : 1; // Debugging Extensions
        UINT32  PSE        : 1; // Page Size Extensions
        UINT32  PAE        : 1; // Physical Address Extension
        UINT32  MCE        : 1; // Machine-Check Enable
        UINT32  PGE        : 1; // Page Global Enable
        UINT32  PCE        : 1; // Performance-Monitoring Counter Enable
        UINT32  OSFXSR     : 1; // OS Support for FXSAVE and FXRSTOR instructions
        UINT32  OSXMMEXCPT : 1; // OS Support for Unmasked SIMD Floating-Point Exceptions
        UINT32  UMIP       : 1; // User-Mode Instruction Prevention
        UINT32  LA57       : 1; // Linear Address 57
        UINT32  VMXE       : 1; // VMX Enable
        UINT32  SMXE       : 1; // SMX Enable
        UINT32  Reserved3  : 1;
        UINT32  FSGSBASE   : 1; // FS/GS Base Access Instructions Enable
        UINT32  PCIDE      : 1; // PCID Enable
        UINT32  OSXSAVE    : 1; // XSAVE and Processor Extended States Enable
        UINT32  Reserved4  : 1;
        UINT32  SMEP       : 1; // Supervisor-Mode Execution Prevention
        UINT32  SMAP       : 1; // Supervisor-Mode Access Prevention
        UINT32  PKE        : 1; // Protection-Key Enable
        UINT32  Reserved5  : 9;
    } Bits;
    UINTN UintN;
} IA32_CR4, CR4_REGISTER;

///////////////////////////////////////////
/// MSRs 
///////////////////////////////////////////

#define MSR_X64_APIC_BASE               0x1B

typedef UINT32 MSR_T;

typedef struct _MSR_RET_T {
    UINT32 Eax; // Lower 32 bits
    UINT32 Edx; // Upper 32 bits
} MSR_RET_T;

typedef union {
    struct {
        UINT32 Reserved1: 11;
        UINT32 NXE      : 1;
        UINT32 Reserved2: 20;
        UINT32 Reserved3: 32;
    } Bits;

    UINT32 Uint32;
    UINT64 Uint64;
} MSR_CORE_IA32_EFER_REGISTER;
///////////////////////////////////////////
///////////////////////////////////////////
#define SIGNATURE_16 EFI_SIGNATURE_16
#define SIGNATURE_32(A,B,C,D) (SIGNATURE_16 (A, B) | (SIGNATURE_16 (C, D) << 16))

///////////////////////////////////////////
/// Interrupt Handling
///////////////////////////////////////////
#define IA32_IDT_GATE_TYPE_INTERRUPT_32  0x8E
#define IA32_IDT_GATE_TYPE_TRAP_32       0x8F

typedef struct {
    UINT16 Isr_low;         // Lower 16 bits of ISR address
    UINT16 Selector;        // Segment selector
    UINT8  Ist;             // Interrupt Stack Table (IST) offset (0 if not used)
    UINT8  Type_attr;       // Type and attributes (e.g., 0x8E for interrupt gate)
    UINT16 Isr_mid;         // Middle 16 bits of ISR address
    UINT32 Isr_high;        // Upper 32 bits of ISR address
    UINT32 Zero;            // Reserved, set to zero
} PACKED IDT_ENTRY, *LPIDT_ENTRY;

typedef struct {
    UINT16 LimitLow;    // Lower 16 bits of the segment limit
    UINT16 BaseLow;     // Lower 16 bits of the base address
    UINT8  BaseMiddle;  // Next 8 bits of the base address
    UINT8  Access;      // Access flags
    UINT8  Granularity; // Granularity and the upper 4 bits of the limit
    UINT8  BaseHigh;    // Upper 8 bits of the base address
} PACKED GDT_ENTRY, *LPGDT_ENTRY;

typedef union {
  struct {
    UINT32    OffsetLow   : 16; ///< Offset bits 15..0.
    UINT32    Selector    : 16; ///< Selector.
    UINT32    Reserved_0  : 8;  ///< Reserved.
    UINT32    GateType    : 8;  ///< Gate Type.  See #defines above.
    UINT32    OffsetHigh  : 16; ///< Offset bits 31..16.
    UINT32    OffsetUpper : 32; ///< Offset bits 63..32.
    UINT32    Reserved_1  : 32; ///< Reserved.
  } Bits;
  struct {
    UINT64    Uint64;
    UINT64    Uint64_1;
  } Uint128;
} IA32_IDT_GATE_DESCRIPTOR;

typedef struct {
    UINT16 Limit;
    UINT64 Base;
} PACKED IDT_DESCRIPTOR, *LPIDT_DESCRIPTOR;

typedef struct {
    UINT16 Limit;
    UINT64 Base;
} PACKED GDT_DESCRIPTOR, *LPGDT_DESCRIPTOR;

typedef struct {
    UINT64 RAX;
    UINT64 RBX;
    UINT64 RCX;
    UINT64 RDX;
    UINT64 RSI;
    UINT64 RDI;
    UINT64 RBP;
    UINT64 R8;
    UINT64 R9;
    UINT64 R10;
    UINT64 R11;
    UINT64 R12;
    UINT64 R13;
    UINT64 R14;
    UINT64 R15;
    UINT64 RSP;
    UINT64 RIP;
    UINT64 CS;
    UINT64 SS;
    UINT64 RFLAGS;
} PACKED CONTEXT, *LPCONTEXT;

EXTERN GLOBAL EFI_LOADED_IMAGE *g_LoadedImage;

EXTERN GLOBAL CONTEXT g_Context;
EXTERN GLOBAL BYTE g_FXSaveArray[512];

EXTERN UINT64 g_ErrorCode;
EXTERN UINT64 g_RAX;
/*
#define __G_CONTEXT_REG(REG) \
    __G_CONTEXT.REG
*/
#define ReadSavedContext(REG) \
    g_Context.REG

#define OverwriteSavedContext(REG) \
    g_Context.REG

VOID EFIAPI InternalHandlerRoutine (VOID);

///////////////////////////////////////////
/// Externs for NASM implementations
///////////////////////////////////////////
//    CONTEXT:        times 128 db 0      ; Space to save general-purpose registers
//    FXSAVE_ARRAY:   times 512 db 0      ; 512 bytes for saving the floating-point/SIMD state
//
//    __ERROR_CODE:   dq 0                ; Error code passed to the interrupt handler
//    __RIP:          dq 0                ; Instruction pointer at the time of the fault
//    __CS:           dq 0                ; Code segment
//    __RFLAGS:       dq 0                ; RFLAGS register
//    __RSP:          dq 0                ; Stack pointer before the interrupt
//    __SS:           dq 0                ; Stack segment
//    __RAX:          dq 0                ; Temporary storage for RAX

///////////////////////////////////////////
/// Intrinsic Functions
///////////////////////////////////////////

NAKED EFI_NORETURN VOID __HaltCpu(
    VOID
);

NAKED UINT16 __ReadCs(
    VOID
);

NAKED VOID __WriteCr0(
    UINT64 Cr0
);

NAKED UINTN __ReadCr0 (
    VOID
);

UINT64 __ReadCr3(
    VOID
);

NAKED UINTN __ReadCr4 (
    VOID
);

MSR_RET_T __ReadMSR(
    UINT32 Msr
);

UINT64 __ReadTSC(
    VOID
);

NAKED VOID InvalidateTLB(
    UINT64 targetAddress
);

BOOLEAN IsPagingAndPageAddressExtensionsEnabled(
    VOID
);

VOID EnableWriteProtectionBit(
    VOID
);

VOID DisableWriteProtectionBit(
    VOID
);

BOOLEAN IsWriteProtectionBitEnabled(
    VOID
);

NAKED BOOLEAN EFIAPI VerifyMSABI(
    UINT64 Rcx  // == 0xdeadbeef
);

BOOLEAN IsAuthenticAMD(
    VOID
);

BOOLEAN IsGenuineIntel(
    VOID
);

LPVOID GetSMMBase(
    VOID
);

#ifdef __NOEXTERNAL_INTRINSICS
NAKED BOOLEAN EFIAPI ProbeAddress(
    UINT64 targetAddress
);

UINT64 ReadSavedContext(
    IN READ_CONTEXT_REG eRegister
);
#else
UNUSED BOOLEAN EFIAPI ProbeAddress(
    UINT64 targetAddress
);

BOOLEAN CheckMSRSupport(VOID);

VOID SYSV_ABI _mm_Fuzz_ReadMSR(
    MSR_T Msr,
    OUT UINT64 *TimeDiff
);

VOID InspectGDTEntry(
    UINT32 uIndex
);
#endif // __NOEXTERNAL_INTRINSICS

UNUSED UINT64 NumberOfUsableBytesInPage(
    UINT64 targetAddress
);

VOID SetIDTEntry(
    INT32 Num,
    UINT64 Base,
    UINT16 Selector,
    UINT8 Flags
);

VOID InstallGP0Handler(
    LPVOID lpHandler
);

VOID InstallGPFaultHandler(
    VOID
);

VOID RestoreGPFaultHandler(
    VOID
);

#endif // _X86INTRIN_H