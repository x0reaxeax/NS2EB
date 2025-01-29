#include <x86intrin.h>
#include <stdfuncs.h>
#include <Screen.h>

extern VOID _mm_InterruptHandler();

GLOBAL ALIGN(0x10) IDT_ENTRY gIDT[256] = { 0 };
GLOBAL IDT_DESCRIPTOR gIDT_PTR = { sizeof(gIDT) - 1, (UINT64) gIDT };

GLOBAL CONTEXT g_Context = { 0 };
GLOBAL BYTE g_FXSaveArray[512] = { 0 };

GLOBAL UINT64 g_ErrorCode = 0;
GLOBAL UINT64 g_RAX = 0;

STATIC GLOBAL IDT_ENTRY gOriginalFaultHandler = { 0 };

STATIC UNUSED NAKED VOID __LoadIDT(
    VOID
) {
    __asm__ __volatile__ (
        "lidt %0;" 
        "ret;"
        : 
        : "memory"(gIDT_PTR)
    );
}

NAKED EFI_NORETURN VOID __HaltCpu(
    VOID
) {
    __asm__ __volatile__ (
        "cli;"
        "hlt;"
    );
}

NAKED UINT16 __ReadCs(
    VOID
) {
    __asm__ __volatile__ (
        "xor %%eax, %%eax;"
        "mov %%cs, %%ax;"
        "ret;"
        :
        :
        : "eax"
    );
}

NAKED VOID __WriteCr0(
    UINT64 Cr0
) {
    __asm__ __volatile__ (
        "mov %0, %%cr0;"
        "ret;"
        :
        : "r" (Cr0)
    );
}

NAKED UINTN __ReadCr0 (
    VOID
) {
    __asm__ __volatile__ (
        "mov %%cr0, %%rax;"
        "ret;"
        :
        :
        : "rax"
    );
}

UINT64 __ReadCr3(
    VOID
) {
    UINT64 Cr3;
    __asm__ __volatile__ (
        "mov %%cr3, %0;"
        : "=r" (Cr3)
    );
    return Cr3;
}

NAKED UINTN __ReadCr4 (
    VOID
) {
    __asm__ __volatile__ (
        "mov %%cr4, %%rax;"
        "ret;"
        :
        :
        : "rax"
    );
}

MSR_RET_T __ReadMSR(
    MSR_T Msr
) {
    MSR_RET_T MsrRet = { 0 };
    __asm__ __volatile__ (
        "mov %1, %%ecx;"
        "rdmsr;"
        "mov %%eax, %0;"
        "mov %%edx, %1;"
        : "=r" (MsrRet.Eax), "=r" (MsrRet.Edx)
        : "r" (Msr)
        : "ecx", "eax", "edx"
    );

    return MsrRet;
}

UINT64 __ReadTSC(
    VOID
) {
    UINT64 Tsc;
    __asm__ __volatile__ (
        "rdtsc;"
        : "=A" (Tsc)
    );
    return Tsc;
}

NAKED VOID InvalidateTLB(
    UINT64 targetAddress
) {
    __asm__ __volatile__ (
        "invlpg (%0);"
        "ret;"
        :
        : "r" (targetAddress)
        : "memory"
    );
}

BOOLEAN IsPagingAndPageAddressExtensionsEnabled(
    VOID
) {
    IA32_CR0 Cr0;
    IA32_CR4 Cr4;

    Cr0.UintN = __ReadCr0();
    Cr4.UintN = __ReadCr4();

    return ((Cr0.Bits.PG != 0) && (Cr4.Bits.PAE != 0));
}

VOID DisableWriteProtectionBit(
    VOID
) {
    IA32_CR0 Cr0;
    Cr0.UintN = __ReadCr0();
    Cr0.Bits.WP = 0;
    __WriteCr0(Cr0.UintN);
}

VOID EnableWriteProtectionBit(
    VOID
) {
    IA32_CR0 Cr0;
    Cr0.UintN = __ReadCr0();
    Cr0.Bits.WP = 1;
    __WriteCr0(Cr0.UintN);
}

BOOLEAN IsWriteProtectionBitEnabled(
    VOID
) {
    IA32_CR0 Cr0;
    Cr0.UintN = __ReadCr0();
    return Cr0.Bits.WP;
}

NAKED BOOLEAN EFIAPI VerifyMSABI(
    UINT64 Rcx  // == 0xdeadbeef
) {
    __asm__ __volatile__ (
        "xor %%eax, %%eax;"
        "cmp $0xdeadbeef, %%ecx;"
        "sete %%al;"
        "ret;"
        :
        :
        : "al"
    );
}

BOOLEAN IsAuthenticAMD(
    VOID
) {
    UINT32 Eax = 0;
    __asm__ __volatile__ (
        "cpuid;"
        : "=a" (Eax)
        : "a" (0x80000002)
    );

    if ((Eax & 0x00FFFFFF) != 0x444D41) {
        return FALSE;
    }

    return TRUE;
}

BOOLEAN IsGenuineIntel(
    VOID
) {
    UINT32 Ebx, Ecx, Edx;
    __asm__ __volatile__ (
        "cpuid;"
        : "=b" (Ebx), "=c" (Ecx), "=d" (Edx)
        : "a" (0) // CPUID leaf 0 returns vendor string in EBX, EDX, ECX
    );

    // "GenuineIntel" in ASCII is split across EBX, EDX, and ECX:
    // EBX = 0x756E6547 ('Genu')
    // EDX = 0x49656E69 ('ineI')
    // ECX = 0x6C65746E ('ntel')

    if (Ebx != 0x756E6547 || Edx != 0x49656E69 || Ecx != 0x6C65746E) {
        return FALSE;
    }

    return TRUE;
}

#ifdef __NOEXTERNAL_INTRINSICS
 NAKED BOOLEAN EFIAPI ProbeAddress(UINT64 targetAddress) {
    //UINT32 uOrirginalValue = *(UINT32*) targetAddress;
    //*(UINT32*) targetAddress = ~uOrirginalValue;

    // dummy write


    __asm__ __volatile__ (
        "mov eax, DWORD PTR [rcx];"
        "mov edx, eax;"
        "not eax;"
        "mov DWORD PTR [rcx], eax;"
        "mov DWORD PTR %[dummyAddress], edx;"
        "clflush [rcx];"
        //"wbinvd;"
        "mov r8d, DWORD PTR [rcx];"
        "mov DWORD PTR [rcx], edx;"
        "xor edx, edx;"
        "cmp r8d, eax;"
        "setz dl;"
        "mov eax, edx;"
        "ret;"
        :
        : [dummyAddress] "m" (aDummyData)
        : "memory", "eax", "edx", "r8"
    );
}

BOOLEAN CheckMSRSupport(VOID) {
    UINT32 Eax = 0;
    __asm__ __volatile__ (
        "cpuid;"
        : "=a" (Eax)
        : "a" (0x1)
    );

    if ((Eax & 0x20) == 0) {
        return FALSE;
    }

    return TRUE;
}

UINT64 ReadSavedContext(
    IN READ_CONTEXT_REG eRegister
) {
    #error "ReadSavedContext is not implemented"
}

LPVOID GetCurrentGDT(VOID) {
    #error "GetCurrentGDT is not implemented"
}

#else 

extern BOOLEAN _mm_CheckMSRSupport(
    VOID
);

extern UINT64 EFIAPI _mm_ReadSavedContext(
    IN READ_CONTEXT_REG eRegister
);

extern LPVOID EFIAPI _mm_GetCurrentGDT(
    OUT LPGDT_DESCRIPTOR
);

extern LPVOID EFIAPI _mm_GetCurrentIDT(
    OUT LPIDT_DESCRIPTOR
);

BOOLEAN CheckMSRSupport(VOID) {
    return _mm_CheckMSRSupport();
}

LPVOID GetCurrentGDT(VOID) {
    GDT_DESCRIPTOR Gdt = { 0 };
    _mm_GetCurrentGDT(&Gdt);

    return (LPVOID) Gdt.Base;
}

LPVOID GetCurrentIDT(VOID) {
    IDT_DESCRIPTOR Idt = { 0 };
    _mm_GetCurrentIDT(&Idt);

    return (LPVOID) Idt.Base;
}
#endif // !__NOEXTERNAL_INTRINSICS

VOID SetIDTEntry(
    INT32 Num,
    UINT64 Base,
    UINT16 Selector,
    UINT8 Flags
) {
    AddLogLineScreen(L"[*] Setting IDT entry..\n");
    LPIDT_ENTRY lpIdtEntry = (LPIDT_ENTRY) GetCurrentIDT();

    lpIdtEntry += Num;

    MemCpy(&gOriginalFaultHandler, lpIdtEntry, sizeof(IDT_ENTRY));

    lpIdtEntry->Isr_low = Base & 0xFFFF;
    lpIdtEntry->Isr_mid = (Base >> 16) & 0xFFFF;
    lpIdtEntry->Isr_high = Base >> 32;
    lpIdtEntry->Selector = Selector;
    lpIdtEntry->Ist = 0; // No IST used
    lpIdtEntry->Type_attr = Flags;
    lpIdtEntry->Zero = 0;
}

VOID InstallInterruptHandler(
    IN LPVOID lpHandler,
    IN EFI_EXCEPTION_TYPE eExceptionType,
    IN UINT8 eGateType
) { 
    SetIDTEntry(
        eExceptionType, 
        (UINT64) lpHandler,
        __ReadCs(),             // Will return "Code segment selector"
        eGateType
    );
    AddLogLineScreen(L"[*] Interrupt Handler: IDT loaded\n");
}

VOID InstallGPFaultHandler(
    VOID
) {
    InstallInterruptHandler(
        _mm_InterruptHandler,
        EXCEPT_X64_GP_FAULT,
        IA32_IDT_GATE_TYPE_INTERRUPT_32     // Present, DPL = 0, 64-bit interrupt gate
    );
}

VOID RestoreGPFaultHandler(
    VOID
) {
    LPIDT_ENTRY lpIdtEntry = (LPIDT_ENTRY) GetCurrentIDT();
    lpIdtEntry += EXCEPT_X64_GP_FAULT;
    MemCpy(lpIdtEntry, &gOriginalFaultHandler, sizeof(IDT_ENTRY));
}
