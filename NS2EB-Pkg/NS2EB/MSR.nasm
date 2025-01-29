BITS 64

global _mm_CheckMSRSupport
global _mm_Fuzz_ReadMSR

section .text

; BOOLEAN CheckMSRSupport(VOID)
_mm_CheckMSRSupport:
    xor eax, eax         ; Clear EAX
    inc eax              ; Set EAX to 1
    cpuid                ; Execute CPUID
    test edx, (1 << 5)   ; Test if bit 5 of EDX is set
    setnz al             ; Set AL to 1 if bit 5 of EDX is set
    ret                  ; Return

; VOID SYSV_ABI _mm_Fuzz_ReadMSR(
;   IN UINT32 Msr,
;   OUT PUINT64 DiffOut
;);
_mm_Fuzz_ReadMSR:
    mov r8, rbx
    
    lfence
    rdtsc
    shl rdx, 32
    or rdx, rax
    mov rbx, rdx
    
    mov ecx, edi
    rdmsr

    lfence
    rdtsc
    shl rdx, 32
    or rdx, rax

    sub rdx, rbx
    mov [rsi], rdx
    
    mov rbx, r8

    ret

