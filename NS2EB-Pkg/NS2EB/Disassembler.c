#include "Disassembler.h"

ZydisMnemonic GetInstructionMnemonic(
    IN LPVOID lpInstructionAddress,
    OUT LPSTR lpBuffer
) {
    ZydisDisassembledInstruction Instruction;
    if (!ZYAN_SUCCESS(ZydisDisassembleIntel(
        ZYDIS_MACHINE_MODE_LONG_64,
        (ZyanU64)lpInstructionAddress,
        lpInstructionAddress,
        0x0F,
        &Instruction
    ))) {
        return ZYDIS_MNEMONIC_INVALID;
    }
    StrCpy8(lpBuffer, Instruction.text);
    return Instruction.info.mnemonic;
}

UINT8 GetInstructionLength(
    IN LPVOID lpInstructionAddress
) {
    ZydisDisassembledInstruction Instruction;
    if (!ZYAN_SUCCESS(ZydisDisassembleIntel(
        ZYDIS_MACHINE_MODE_LONG_64,
        (ZyanU64)lpInstructionAddress,
        lpInstructionAddress,
        0x0F,
        &Instruction
    ))) {
        return 0;
    }
    return Instruction.info.length;
}

VOID DisassembleFunction(LPVOID lpFunction) {
    ZyanUSize offset = 0;
    ZydisDisassembledInstruction Instruction;

    CONST UINTN maxByteCount = 0x180;               // Maximum number of bytes to disassemble
    CONST INT32 addressWidth = 16;                  // Width for the address
    CONST INT32 opcodeBytesWidth = (15 * 2) + 14;   // Width for the bytes column
    MAYBE_UNUSED CONST INT32 mnemonicWidth = 32;    // Width for the mnemonic column

    while (ZYAN_SUCCESS(ZydisDisassembleIntel(
        ZYDIS_MACHINE_MODE_LONG_64,
        (ZyanU64)lpFunction + offset,
        lpFunction + offset,
        maxByteCount - offset,
        &Instruction
    ))) {
        // Print the address
        Print(L"0x%0*llx: |", addressWidth, (ZyanU64)lpFunction + offset);

        // Print the instruction bytes
        int bytesPrinted = 0;
        for (ZyanUSize i = 0; i < Instruction.info.length; ++i) {
            Print(L" %02x", ((VOLATILE UINT8*)lpFunction)[offset + i]);
            bytesPrinted++;
        }

        // Calculate the number of spaces needed to align the mnemonic column
        int usedBytesSpace = bytesPrinted * 3;  // 3 = 2 characters for byte + 1 space
        int bytesSpace = (usedBytesSpace < opcodeBytesWidth) ? (opcodeBytesWidth - usedBytesSpace) : 1;
        for (INT32 i = 0; i < bytesSpace; i++) {
            Print(L" ");
        }

        // Print the mnemonic (instruction string)
        Print(L"| %a", Instruction.text);

#if (PRINT_INSTRUCTION_LENGTH)
        // Calculate the number of spaces needed after the mnemonic
        UINTN instStrLen = StrLen8(Instruction.text);
        INT32 mnemonicSpace = mnemonicWidth - (UINT32) instStrLen;
        if (mnemonicSpace < 0) {
            mnemonicSpace = 1;  // Ensure at least one space for separation
        }
        for (INT32 i = 0; i < mnemonicSpace; i++) {
            Print(L" ");
        }

        // Print the length of the instruction
        Print(L"| [LEN: 0x%02x]\n", Instruction.info.length);
#else
        Print(L"\n");
#endif
        // Check if the instruction is 'RET', and break if it is
        if (ZYDIS_MNEMONIC_RET ==  Instruction.info.mnemonic) {
            break;
        }

        // Move to the next instruction
        offset += Instruction.info.length;
    }
}