#ifndef _DISASSEMBLER_WRAPPER_H
#define _DISASSEMBLER_WRAPPER_H

#include "Zydis.h"
#include "stdfuncs.h"
#include "x86intrin.h"

#define PRINT_INSTRUCTION_LENGTH    0


ZydisMnemonic GetInstructionMnemonic(
    IN LPVOID lpInstructionAddress,
    OUT LPSTR lpBuffer
);

UINT8 GetInstructionLength(
    LPVOID lpInstructionAddress
);

VOID DisassembleFunction(
    LPVOID lpFunction
);

#endif