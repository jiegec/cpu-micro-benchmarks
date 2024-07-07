;          testmemcpyal.nasm                         Agner Fog 2013-08-07

; Test if memory copying has penalty for false dependence between 
; source and destination addressses
; Timing for different alignments

; Instructions: Compile on 64 bit platform and link with testmemcpyal.nasm
; The testmemcpy functions have these restrictions:
; 'count' must be divisible by the operand size
; 'destination' must be aligned by the operand size

default rel

global testmemcpy0           ; copy memory with rep movsq instruction
global testmemcpy4           ; copy memory with 4 bytes operands
global testmemcpy8           ; copy memory with 8 bytes operands
global testmemcpy16          ; copy memory with 16 bytes operands
global testmemcpy32          ; copy memory with 32 bytes operands

global cpuid_ex              ; cpuid
global ReadTSC               ; read time stamp counter


SECTION .text  align=16

; C++ prototype:
; extern "C" void testmemcpy0(void * dest, const void * src, size_t count);
; extern "C" void testmemcpy4(void * dest, const void * src, size_t count);
; extern "C" void testmemcpy8(void * dest, const void * src, size_t count);
; extern "C" void testmemcpy16(void * dest, const void * src, size_t count);
; extern "C" void testmemcpy32(void * dest, const void * src, size_t count);

; function prolog
; dest = rdi, src = rsi, count = rcx
%MACRO  PROLOGM 0
%IFDEF  WINDOWS
        push    rdi
        push    rsi
        mov     rdi, rcx
        mov     rsi, rdx
        mov     rcx, r8
%ELSE
        mov     rcx, rdx
%ENDIF
%ENDMACRO

; function epilog
%MACRO  EPILOGM 0
%IFDEF  WINDOWS
        pop     rsi
        pop     rdi
%ENDIF
        ret
%ENDMACRO

; memcpy test versions
testmemcpy0:
        PROLOGM
        shr     ecx, 3
        rep     movsq
        EPILOGM

testmemcpy4:
        PROLOGM
        and     rcx, -4
        add     rdi, rcx
        add     rsi, rcx
        neg     rcx
L400:   mov     eax, [rsi+rcx]
        mov     [rdi+rcx], eax
        add     rcx, 4
        jnz     L400
        EPILOGM

testmemcpy8:
        PROLOGM
        and     rcx, -8
        add     rdi, rcx
        add     rsi, rcx
        neg     rcx
L800:   mov     rax, [rsi+rcx]
        mov     [rdi+rcx], rax
        add     rcx, 8
        jnz     L800
        EPILOGM

testmemcpy16:
        PROLOGM
        and     rcx, -16
        add     rdi, rcx
        add     rsi, rcx
        neg     rcx
L1600:  movups  xmm0, [rsi+rcx]
        movaps  [rdi+rcx], xmm0
        add     rcx, 16
        jnz     L1600
        EPILOGM

testmemcpy32:
        PROLOGM
        and     rcx, -32
        add     rdi, rcx
        add     rsi, rcx
        neg     rcx
L3200:  vmovups ymm0, [rsi+rcx]
        vmovaps [rdi+rcx], ymm0
        add     rcx, 32
        jnz     L3200
        vzeroupper
        EPILOGM




; ********** cpuid_ex function **********
; C++ prototype:
; extern "C" void cpuid_ex (int abcd[4], int a, int c);
; Input: a = eax, c = ecx
; Output: abcd[0] = eax, abcd[1] = ebx, abcd[2] = ecx, abcd[3] = edx

cpuid_ex:
%IFDEF   WINDOWS
; parameters: rcx = abcd, edx = a, r8d = c
        push    rbx
        xchg    rcx, r8
        mov     eax, edx
        cpuid                          ; input eax, ecx. output eax, ebx, ecx, edx
        mov     [r8],    eax
        mov     [r8+4],  ebx
        mov     [r8+8],  ecx
        mov     [r8+12], edx
        pop     rbx
%ELSE
; parameters: rdi = abcd, esi = a, edx = c
        push    rbx
        mov     eax, esi
        mov     ecx, edx
        cpuid                          ; input eax, ecx. output eax, ebx, ecx, edx
        mov     [rdi],    eax
        mov     [rdi+4],  ebx
        mov     [rdi+8],  ecx
        mov     [rdi+12], edx
        pop     rbx
%ENDIF        
        ret
;cpuid_ex END


; ********** ReadTSC function **********
; C++ prototype:
; extern "C" __int64 ReadTSC (void);

ReadTSC:
        push    rbx                    ; ebx is modified by cpuid
        sub     eax, eax               ; 0
        cpuid                          ; serialize
        rdtsc                          ; read time stamp counter into edx:eax
        shl     rdx, 32
        or      rax, rdx               ; combine into 64 bit register        
        push    rax
        sub     eax, eax
        cpuid                          ; serialize
        pop     rax                    ; return value
        pop     rbx
        ret
;ReadTSC ENDP

