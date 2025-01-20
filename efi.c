#define GNU_EFI_USE_MS_ABI 1
#define MS_ABI __attribute__((ms_abi))

#include <efi/efi.h>
#include <efi/efilib.h>

#include "x86intrin.h"
#include "stdfuncs.h"
#include "Disassembler.h"
#include "Screen.h"

#define NEED_MORE_SPEED

#define MSR_MIN 0x0
#define MAX_MSR 0x100000

GLOBAL EFI_LOADED_IMAGE *g_LoadedImage = NULL;

GLOBAL VOLATILE UINT32 g_ExecFlag = 1;          // Control flag for MSR fuzzing
GLOBAL VOLATILE UINT32 g_InterruptFlag = 0;     // Indicates whether the interrupt handler was called

VOID EFIAPI InternalHandlerRoutine (VOID) {
    UINT64 Rip = ReadSavedContext(RIP);

    UINT8 uInstructionSize = GetInstructionLength(
        (LPVOID) Rip
    );
    
    if (0 == uInstructionSize) {
        Print(L"[-] Failed to get instruction size\n");
        Print(L"[!] Halting the CPU\n");
        __HaltCpu();
    }
    
    CHAR szInstruction[64] = { 0 };
    ZydisMnemonic eMnemonic = GetInstructionMnemonic(
        (LPVOID) Rip,
        szInstruction
    );

    if (ZYDIS_MNEMONIC_INVALID == eMnemonic) {
        Print(L"[-] Failed to get instruction mnemonic\n");
        Print(L"[!] Halting the CPU\n");
        __HaltCpu();
    }

    if (eMnemonic != ZYDIS_MNEMONIC_RDMSR) {
        Print(L"[*] GPFault trigger instruction: '%a'\n", szInstruction);
        Print(L"[*] Halting the CPU\n");
        __HaltCpu();
    }

    OverwriteSavedContext(RIP) += uInstructionSize;
}

UINT64 GetAverageMSRExecutionTime(
    UINT32 uIterations,
    MSR_T Msr
) {
    UINT64 uTotalTime = 0;
    for (UINT32 i = 0; i < uIterations; i++) {
        PrintAt(0, OUTPUT_CURRENT_MSR, L"[*] Current MSR: 0x%02llX\n", Msr);
        UINTN TimeDiff = 0;
        _mm_Fuzz_ReadMSR(Msr, &TimeDiff);
        uTotalTime += TimeDiff;
    }

    ClearLine(OUTPUT_CURRENT_MSR);

    return uTotalTime / uIterations;
}

VOID FuzzMSRs(
    VOID
) {
    MSR_T Msr;
    UINT64 TimeDiff = 0;

    for (Msr = MSR_MIN; Msr < MAX_MSR && (VOLATILE UINT32) g_ExecFlag; Msr++) {
#ifdef NEED_MORE_SPEED
        if (0 == (Msr % 0x1000)) {
            PrintAt(0, OUTPUT_CURRENT_MSR, L"[*] MSR Checkpoint: 0x%08X\n", Msr);
        }
#else
        PrintAt(0, OUTPUT_CURRENT_MSR, L"[*] Current MSR: 0x%08X\n", Msr);
#endif
        _mm_Fuzz_ReadMSR(Msr, &TimeDiff);

        AddLogLineFile(
            L"%c=0x%02x;T=0x%04llx\n",
            (g_InterruptFlag) ? L'I' : L'V',
            Msr, 
            TimeDiff
        );
        TimeDiff = 0;
        g_InterruptFlag = 0;
    }
    CHAR16 *Reason = (!g_ExecFlag) ? L"(Aborted)" : L"";
    AddLogLineScreen(L"[+] Fuzzing complete %s\n", Reason);
}

EFI_STATUS OpenLog(
    CHAR16 *FileName
) {
    EFI_FILE_PROTOCOL *Root;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileSystem;
    EFI_STATUS Status;

    Status = uefi_call_wrapper(
        BS->HandleProtocol,
        3,
        g_LoadedImage->DeviceHandle,
        &gEfiSimpleFileSystemProtocolGuid,
        (void **)&FileSystem
    );

    if (EFI_ERROR(Status)) {
        return Status;
    }

    Status = uefi_call_wrapper(
        FileSystem->OpenVolume,
        2,
        FileSystem,
        &Root
    );

    if (EFI_ERROR(Status)) {
        return Status;
    }

    Status = uefi_call_wrapper(
        Root->Open,
        5,
        Root,
        &g_LogFile,
        FileName,
        EFI_FILE_MODE_CREATE | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_READ,
        0
    );

    return Status;
}

EFI_STATUS CloseLog(
    VOID
) {
    if (NULL == g_LogFile) {
        return EFI_INVALID_PARAMETER;
    }

    return uefi_call_wrapper(
        g_LogFile->Close,
        1,
        g_LogFile
    );
}

EFI_STATUS efi_main(
    EFI_HANDLE ImageHandle, 
    EFI_SYSTEM_TABLE *SystemTable
) {
    EFI_STATUS Status = EFI_SUCCESS;
  
    Status = uefi_call_wrapper(
        SystemTable->BootServices->HandleProtocol,
        3,
        ImageHandle,
        &gEfiLoadedImageProtocolGuid,
        (void **) &g_LoadedImage
    );
    
    if (EFI_ERROR(Status)) {
        Print(L"[-] Failed to get LoadedImageProtocol: E%lx\n", Status);
        return Status;
    }

    Print(L"[*] Hello, UEFI!\n");
    Print(L"[*] ImageBase: 0x%llx\n", g_LoadedImage->ImageBase);

    BOOLEAN MsrSupport = CheckMSRSupport();
    if (!MsrSupport) {
        Print(L"[-] No MSR support detected\n");
        return EFI_UNSUPPORTED;
    }

    BOOLEAN AuthenticAMD = IsAuthenticAMD();
    BOOLEAN GenuineIntel = IsGenuineIntel();

    if (AuthenticAMD) {
        Print(L"[+] Authentic AMD CPU detected\n");
    } else if (GenuineIntel) {
        Print(L"[+] Genuine Intel CPU detected\n");
    } else {
        Print(L"[-] Unknown CPU, AliExpress enjoyer perhaps?\n");
    }

    BOOLEAN PagingEnabled = IsPagingAndPageAddressExtensionsEnabled();
    Print(L"[*] Paging and PAE enabled: %d\n", PagingEnabled);

    DisableWriteProtectionBit();
    Print(L"[+] Write protection bit disabled\n"); 
    
    Status = OpenLog(LOGFILE_NAME);
    if (EFI_ERROR(Status)) {
        Print(L"[-] Failed to open log file: E%lx\n", Status);
        return Status;
    }

    g_TempMemory1 = AllocateZeroPool(0x1000);
    g_TempMemory2 = AllocateZeroPool(0x1000);
    if (NULL == g_TempMemory1 || NULL == g_TempMemory2) {
        CloseLog();
        Print(L"[-] Failed to allocate memory\n");
        return EFI_OUT_OF_RESOURCES;
    }

    Print(
        L"[==============================]\n"
        L"[ ***<<< NS2EB MSR FUZZ >>>*** ]\n"
        L"[==============================]\n"
        L"[*] Press any key to start..\n"
    );
    WaitForSingleEvent(ST->ConIn->WaitForKey, 0);


    if (EFI_SUCCESS != InitializeScreen()) {
        Print(L"[-] Failed to initialize screen\n");
        CloseLog();
        return EFI_UNSUPPORTED;
    }

    PrintAt(
        0, 
        OUTPUT_EXECFLAG_ADDR, 
        L"[*] ExecFlag: 0x%llx\n", 
        &g_ExecFlag
    );

    InstallGPFaultHandler();
    AddLogLineScreen(L"[+] Installed GPFault handler\n");

    AddLogLineScreen(L"[*] Testing GPFault handler..\n");
    __ReadMSR(0xFFFFFFFF);
    AddLogLineScreen(L"[+] GPFault handler successfully installed\n");
    AddLogLineScreen(L"[*] Starting MSR fuzzing..\n");

    
    AddLogLineScreen(L"[*] Testing average MSR read cycles..\n");
    UINT64 AvgExecTimeBad = GetAverageMSRExecutionTime(
        100,
        0xFFFFFFFF
    );
    UINT64 AvgExecTimeGood = GetAverageMSRExecutionTime(
        100,
        MSR_X64_APIC_BASE
    );

    AddLogLineScreen(L"[*] Average Invalid MSR read cycles: %llu\n", AvgExecTimeBad);
    AddLogLineScreen(L"[*] Average Valid MSR read cycles:   %llu\n", AvgExecTimeGood);

    DisplayAverageMSRTime(AvgExecTimeBad, FALSE);
    DisplayAverageMSRTime(AvgExecTimeGood, TRUE);

    FuzzMSRs();
    // wait for input
    WaitForSingleEvent(ST->ConIn->WaitForKey, 0);

    EnableWriteProtectionBit();

    // wait for a key press
    WaitForSingleEvent(ST->ConIn->WaitForKey, 0);

    CloseLog();
    FreePool(g_TempMemory1);
    FreePool(g_TempMemory2);

    // Restore the original GPFault handler
    RestoreGPFaultHandler();

    return EFI_SUCCESS;
}