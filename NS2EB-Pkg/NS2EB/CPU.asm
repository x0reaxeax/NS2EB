BITS 64
DEFAULT REL

%define RDMSR_OPCODE_U16 0x320F

global _mm_InterruptHandler
global _mm_ReadSavedContext
global _mm_GetCurrentGDT
global _mm_GetCurrentIDT

struc CONTEXT_AMD64
    .RAX    resq 1
    .RBX    resq 1
    .RCX    resq 1
    .RDX    resq 1
    .RSI    resq 1
    .RDI    resq 1
    .RBP    resq 1
    .R8     resq 1
    .R9     resq 1
    .R10    resq 1
    .R11    resq 1
    .R12    resq 1
    .R13    resq 1
    .R14    resq 1
    .R15    resq 1
    .RSP    resq 1
    .RIP    resq 1
    .CS     resq 1
    .SS     resq 1
    .RFLAGS resq 1
endstruc

extern g_Context            ; For saving context inside the interrupt handler
extern g_FXSaveArray        ; For saving floating-point and SIMD state
extern g_ErrorCode          ; Handler error code
extern g_RAX                ; Temporary storage for RAX
extern g_InterruptFlag      ; For indicating whether the interrupt handler handled an exception

section .text
    ; VOID EFIAPI InternalHandlerRoutine (
    ;     IN UINT64 ErrorCode,
    ;     IN UINT64 Rip
    ; );
    extern InternalHandlerRoutine       ; Declare the external C internal handler routine


; LPVOID EFIAPI _mm_GetCurrentGDT(
;    OUT LPGDT_DESCRIPTOR
; );
_mm_GetCurrentGDT:
    ; Load the GDT descriptor into RCX
    sgdt [rcx]
    mov rax, rcx
    ret

; LPVOID EFIAPI _mm_GetCurrentIDT(
;    OUT LPIDT_DESCRIPTOR
; );
_mm_GetCurrentIDT:
    ; Load the IDT descriptor into RCX
    sidt [rcx]
    mov rax, rcx
    ret

; UINT64 EFIAPI ReadSavedContext(
;     IN READ_CONTEXT_REG eRegister
; );
_mm_ReadSavedContext:
    ; Load the register index into RAX
    mov rax, rcx

    ; Load the register value from the context
    mov rcx, g_Context
    mov rax, [rcx + rax * 8]
    ret

SaveContext:
    mov [g_Context + CONTEXT_AMD64.RAX], rax
    mov [g_Context + CONTEXT_AMD64.RBX], rbx
    mov [g_Context + CONTEXT_AMD64.RCX], rcx
    mov [g_Context + CONTEXT_AMD64.RDX], rdx
    mov [g_Context + CONTEXT_AMD64.RSI], rsi
    mov [g_Context + CONTEXT_AMD64.RDI], rdi
    mov [g_Context + CONTEXT_AMD64.RBP], rbp
    mov [g_Context + CONTEXT_AMD64.R8],  r8
    mov [g_Context + CONTEXT_AMD64.R9],  r9
    mov [g_Context + CONTEXT_AMD64.R10], r10
    mov [g_Context + CONTEXT_AMD64.R11], r11
    mov [g_Context + CONTEXT_AMD64.R12], r12
    mov [g_Context + CONTEXT_AMD64.R13], r13
    mov [g_Context + CONTEXT_AMD64.R14], r14
    mov [g_Context + CONTEXT_AMD64.R15], r15

    ; Save the floating-point and SIMD state
    fxsave [g_FXSaveArray]
    ret

RestoreContext:
    ; Restore the floating-point and SIMD state
    fxrstor [g_FXSaveArray]

    ; Restore general-purpose registers
    mov r15, [g_Context + CONTEXT_AMD64.R15]
    mov r14, [g_Context + CONTEXT_AMD64.R14]
    mov r13, [g_Context + CONTEXT_AMD64.R13]
    mov r12, [g_Context + CONTEXT_AMD64.R12]
    mov r11, [g_Context + CONTEXT_AMD64.R11]
    mov r10, [g_Context + CONTEXT_AMD64.R10]
    mov r9,  [g_Context + CONTEXT_AMD64.R9]
    mov r8,  [g_Context + CONTEXT_AMD64.R8]
    mov rbp, [g_Context + CONTEXT_AMD64.RBP]
    mov rdi, [g_Context + CONTEXT_AMD64.RDI]
    mov rsi, [g_Context + CONTEXT_AMD64.RSI]
    mov rdx, [g_Context + CONTEXT_AMD64.RDX]
    mov rcx, [g_Context + CONTEXT_AMD64.RCX]
    mov rbx, [g_Context + CONTEXT_AMD64.RBX]
    mov rax, [g_Context + CONTEXT_AMD64.RAX]
    ret

_mm_InterruptHandler:
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ;;; Main interrupt handler routine
    ;;; This routine is called by the interrupt stub in the interrupt descriptor
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; Stack layout on entry:
    ; [RSP+0] : Error code (for exceptions that push an error code)
    ; [RSP+8] : RIP
    ; [RSP+16]: CS
    ; [RSP+24]: RFLAGS
    ; [RSP+32]: RSP (before the interrupt)
    ; [RSP+40]: SS
    
    ; Disable interrupts and pray
    cli
    
    sub rdx, rbx
    mov [rsi], rdx
    ; check if these modifcations keep InternalHandlerRoutine working and able to get MSR value

    ; Save RAX 
    mov [g_RAX], rax

    ; Save passed arguments
    mov rax, [rsp + 0]        ; Load error code into RAX
    mov [g_ErrorCode], rax

    mov rax, [rsp + 8]        ; Load RIP into RAX
    mov [g_Context + CONTEXT_AMD64.RIP], rax

    mov rax, [rsp + 16]       ; Load CS into RAX
    mov [g_Context + CONTEXT_AMD64.CS], rax

    mov rax, [rsp + 24]       ; Load RFLAGS into RAX
    mov [g_Context + CONTEXT_AMD64.RFLAGS], rax

    mov rax, [rsp + 32]       ; Load RSP into RAX
    mov [g_Context + CONTEXT_AMD64.RSP], rax

    mov rax, [rsp + 40]       ; Load SS into RAX
    mov [g_Context + CONTEXT_AMD64.SS], rax

    ; Restore RAX
    mov rax, [g_RAX]

    ; Save the context
    call SaveContext

%ifdef ZYDIS_DISASM_BACKING
    ; Call the internal routine
    call InternalHandlerRoutine

    ; Check if RIP has been updated
    mov rax, [g_Context + CONTEXT_AMD64.RIP]
    cmp rax, [rsp + 8]
    jne .ignore_fault

    ; No update, halt
    hlt
%else
    ; Check for RDMSR internally
    mov rax, [g_Context + CONTEXT_AMD64.RIP]
    movzx eax, WORD [rax]
    cmp eax, RDMSR_OPCODE_U16
    je .update_rip

    ; not RDMSR, halt
    hlt

.update_rip:
    ; Update RIP to skip the RDMSR instruction
    add QWORD [g_Context + CONTEXT_AMD64.RIP], 2
    mov rax, [g_Context + CONTEXT_AMD64.RIP]
%endif

.ignore_fault:
    ; Update return address
    mov [rsp + 8], rax
    
    ; Set the exception indicator flag
    xor eax, eax
    inc eax
    mov DWORD [g_InterruptFlag], eax

    ; Restore the context
    call RestoreContext

    ; Enable interrupts
    sti
    ; Remove the error code from the stack
    add rsp, 8

    ; Return from interrupt
    iretq