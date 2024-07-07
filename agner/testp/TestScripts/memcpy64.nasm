;*************************  memcpy64.asm  ************************************
; Author:           Agner Fog
; Date created:     2008-07-19
; Last modified:    2016-11-03
; Description:
; Faster version of the standard memcpy function:
; void * A_memcpy(void * dest, const void * src, size_t count);
; Copies 'count' bytes from 'src' to 'dest'
;
; Calling convention: 64 bit Linux or Windows,
; Stack alignment is not required. No shadow space or red zone used.
; May be called internally from strcpy and strcat without stack aligned.
;
; Optimization:
; Different versions for testing which implementation is fastest.
;
; (c) 2008-2016 by Agner Fog. GNU General Public License www.gnu.org/licenses
;******************************************************************************


; Alternative implementations of memcpy
GLOBAL MEMCPYR                         ; Use REP MOVS instruction
GLOBAL MEMCPYS2                        ; Use SSE2 instruction set, 16 bytes aligned load/stores
GLOBAL MEMCPYS3                        ; Use SSSE3 instruction set, 16 bytes aligned load/stores
GLOBAL MEMCPYU                         ; Use unaligned 16 bytes loads, aligned stores
GLOBAL MEMCPYNT                        ; Use non-temporal stores, 16 bytes
GLOBAL MEMCPYNT32                      ; Use non-temporal stores, 32 bytes
GLOBAL MEMCPYAVXA                      ; Use AVX instruction set, 16 bytes aligned load/stores
GLOBAL MEMCPYAVXU                      ; Use AVX instruction set, 32 bytes unaligned loads, aligned stores
GLOBAL MEMCPYXOP                       ; Use AMD XOP instruction set, 16 bytes aligned load/stores
GLOBAL MEMCPYAVX512F                   ; Use AVX512F  instruction set, 64 bytes unaligned loads, aligned stores
GLOBAL MEMCPYAVX512BW                  ; Use AVX512BW instruction set, 64 bytes unaligned loads, aligned stores


; Define prolog and epilog for this function
%MACRO  PROLOG  0
%IFDEF  WINDOWS
        push    rsi
        push    rdi
        mov     rdi, rcx
        mov     rsi, rdx
        mov     rdx, r8
%ENDIF
%ENDM

%MACRO  EPILOG  0
%IFDEF  WINDOWS
        pop     rdi
        pop     rsi
%ENDIF
        mov     rax, r9                ; Return value = dest
        ret
%ENDM


SECTION .text  align=16
default rel

; 80386 version used when SSE2 not supported:
MEMCPYR:PROLOG
; rdi = dest
; rsi = src
; edx = count
        mov     rcx, rdx
        cld
        mov     r9, rdi
        cmp     ecx, 8
        jb      G500
G100:   test    edi, 1
        jz      G200
        movsb
        dec     ecx
G200:   test    edi, 2
        jz      G300
        movsw
        sub     ecx, 2
G300:   test    edi, 4
        jz      G400
        movsd
        sub     ecx, 4
G400:   ; rdi is aligned now
        mov     edx, ecx
        shr     ecx, 3
        rep     movsq                  ; move 8 bytes at a time
        mov     ecx, edx
        and     ecx, 7
        rep     movsb                  ; move remaining 0-7 bytes
        EPILOG
        
G500:   ; count < 8. Move one byte at a time
        rep     movsb                  ; move count bytes
        EPILOG
        
        
align 16        
MEMCPYU:PROLOG
        mov     rcx, rdx
        mov     r9, rdi
        cmp     rcx, 40H
        jb      B80                   ; Use simpler code if count < 64
        ; Calculate size of first block up to first regular boundary of dest
        mov     edx, edi
        neg     edx
        and     edx, 0FH
        jz      U300                    ; Skip if dest aligned by 16
        ; rdx = size of first partial block, 1 - 15 bytes
        add     rsi, rdx
        add     rdi, rdx
        sub     rcx, rdx
        neg     rdx
        cmp     edx, -8
        jg      U200
        ; move 8 bytes
        mov     rax, [rsi+rdx]
        mov     [rdi+rdx], rax
        add     rdx, 8
U200:   cmp     edx, -4        
        jg      U210
        ; move 4 bytes
        mov     eax, [rsi+rdx]
        mov     [rdi+rdx], eax
        add     rdx, 4
U210:   cmp     edx, -2        
        jg      U220
        ; move 2 bytes
        movzx   eax, word [rsi+rdx]
        mov     [rdi+rdx], ax
        add     rdx, 2
U220:   cmp     edx, -1
        jg      U300
        ; move 1 byte
        movzx   eax, byte [rsi+rdx]
        mov     [rdi+rdx], al
        
U300:   ; Now dest is aligned by 16. Any partial block has been moved        
        mov     rdx, rcx
        and     rdx, -10H   ; size of regular piece to move
        jz      U500
        add     rsi, rdx    ; point to end
        add     rdi, rdx
        neg     rdx
align 16        
U400:   ; loop for 16-bytes blocks, destination aligned
        movdqu  xmm0, [rsi+rdx]
        movdqa  [rdi+rdx], xmm0
        add     rdx, 10H
        jnz     U400
U500:   ; any remaining data after main loop
        and     ecx, 0FH
        jz      U900
        test    ecx, 8
        jz      U600
        mov     rax, [rsi]
        mov     [rdi], rax
        add     rsi, 8
        add     rdi, 8
U600:   
        test    ecx, 4
        jz      U700
        mov     eax, [rsi]
        mov     [rdi], eax
        add     rsi, 4
        add     rdi, 4        
U700:   
        test    ecx, 2
        jz      U800
        movzx   eax, word [rsi]
        mov     [rdi], ax
        add     rsi, 2
        add     rdi, 2
U800:   
        test    ecx, 1
        jz      U900
        movzx   eax, byte [rsi]
        mov     [rdi], al
       ; add     rsi, 1
       ; add     rdi, 1
U900:   EPILOG

      
align 16        
MEMCPYAVXU:
        PROLOG
        mov     rcx, rdx
        mov     r9,  rdi
        cmp     rcx, 40H
        jb      B80                   ; Use simpler code if count < 64
        ; Calculate size of first block up to first 32 bytes boundary of dest
        mov     edx, edi
        neg     edx
        and     edx, 1FH
        jz      V300                    ; Skip if dest aligned by 32
        ; rdx = size of first partial block, 1 - 31 bytes
        add     rsi, rdx
        add     rdi, rdx
        sub     rcx, rdx
        neg     rdx
        
        cmp     edx, -16
        jg      V200
        ; move 16 bytes
        vmovdqu xmm0, [rsi+rdx]
        vmovdqu [rdi+rdx], xmm0
        add     rdx, 16
V200:   cmp     edx, -8
        jg      V210
        ; move 8 bytes
        mov     rax, [rsi+rdx]
        mov     [rdi+rdx], rax
        add     rdx, 8
V210:   cmp     edx, -4        
        jg      V220
        ; move 4 bytes
        mov     eax, [rsi+rdx]
        mov     [rdi+rdx], eax
        add     rdx, 4
V220:   cmp     edx, -2        
        jg      V230
        ; move 2 bytes
        movzx   eax, word [rsi+rdx]
        mov     [rdi+rdx], ax
        add     rdx, 2
V230:   cmp     edx, -1
        jg      V300
        ; move 1 byte
        movzx   eax, byte [rsi+rdx]
        mov     [rdi+rdx], al
        
V300:   ; Now dest is aligned by 32. Any partial block has been moved        
        mov     rdx, rcx
        and     rdx, -20H   ; size of regular piece to move
        jz      V500
        add     rsi, rdx    ; point to end
        add     rdi, rdx
        neg     rdx
align 16        
V400:   ; loop for 32-bytes blocks, destination aligned
        vmovdqu ymm0, [rsi+rdx]
        vmovdqa [rdi+rdx], ymm0
        add     rdx, 20H
        jnz     V400
V500:   ; any remaining data after main loop
        and     ecx, 1FH
        jz      V900         
        test    ecx, 10H
        jz      V610
        vmovdqu xmm0, [rsi]
        vmovdqa [rdi], xmm0
        add     rsi, 10H
        add     rdi, 10H
V610:   
        test    ecx, 8
        jz      V620
        mov     rax, [rsi]
        mov     [rdi], rax
        add     rsi, 8
        add     rdi, 8
V620:   
        test    ecx, 4
        jz      V630
        mov     eax, [rsi]
        mov     [rdi], eax
        add     rsi, 4
        add     rdi, 4        
V630:   
        test    ecx, 2
        jz      V640
        movzx   eax, word [rsi]
        mov     [rdi], ax
        add     rsi, 2
        add     rdi, 2
V640:   
        test    ecx, 1
        jz      V900
        movzx   eax, byte [rsi]
        mov     [rdi], al
       ; add     rsi, 1
       ; add     rdi, 1
V900:   vzeroupper
        EPILOG
        

align 16        
MEMCPYNT32:     ; same as MEMCPYAVXU, but with non-temporal stores
        PROLOG
        mov     rcx, rdx
        mov     r9,  rdi
        cmp     rcx, 40H
        jb      B80                   ; Use simpler code if count < 64
        ; Calculate size of first block up to first 32 bytes boundary of dest
        mov     edx, edi
        neg     edx
        and     edx, 1FH
        jz      W300                    ; Skip if dest aligned by 32
        ; rdx = size of first partial block, 1 - 31 bytes
        add     rsi, rdx
        add     rdi, rdx
        sub     rcx, rdx
        neg     rdx
        
        cmp     edx, -16
        jg      W200
        ; move 16 bytes
        vmovdqu xmm0, [rsi+rdx]
        vmovdqu [rdi+rdx], xmm0
        add     rdx, 16
W200:   cmp     edx, -8
        jg      W210
        ; move 8 bytes
        mov     rax, [rsi+rdx]
        mov     [rdi+rdx], rax
        add     rdx, 8
W210:   cmp     edx, -4        
        jg      W220
        ; move 4 bytes
        mov     eax, [rsi+rdx]
        mov     [rdi+rdx], eax
        add     rdx, 4
W220:   cmp     edx, -2        
        jg      W230
        ; move 2 bytes
        movzx   eax, word [rsi+rdx]
        mov     [rdi+rdx], ax
        add     rdx, 2
W230:   cmp     edx, -1
        jg      W300
        ; move 1 byte
        movzx   eax, byte [rsi+rdx]
        mov     [rdi+rdx], al
        
W300:   ; Now dest is aligned by 32. Any partial block has been moved        
        mov     rdx, rcx
        and     rdx, -20H   ; size of regular piece to move
        jz      V500
        add     rsi, rdx    ; point to end
        add     rdi, rdx
        neg     rdx
align 16        
W400:   ; loop for 32-bytes blocks, destination aligned
        vmovups ymm0, [rsi+rdx]
        vmovntps [rdi+rdx], ymm0
        add     rdx, 20H
        jnz     W400
        jmp     V500




; extern "C" void * A_memcpy(void * dest, const void * src, size_t count);
;       registers: 
;       rdi = destination
;       rsi = source
;       rcx = count

; Function entry:
MEMCPYS2:
        PROLOG
        lea    r8, [AlignmentDispatchSSE2]
        jmp    MEMCPY_COMMON
MEMCPYS3:
        PROLOG
        lea    r8, [AlignmentDispatchSupSSE3]
        jmp    MEMCPY_COMMON
MEMCPYAVXA:
        PROLOG        
        lea    r8, [AlignmentDispatchAVXA]
        jmp    MEMCPY_COMMON        
MEMCPYNT:
        PROLOG
        lea    r8, [AlignmentDispatchNT]
;        jmp    MEMCPY_COMMON
MEMCPY_COMMON:
        mov     rcx, rdx
        mov     r9,  rdi               ; dest
;$memcpyEntry2:

        cmp     rcx, 40H
        jae     B100                   ; Use simpler code if count < 64
        
        ; count < 64. Move 32-16-8-4-2-1 bytes
B80:    add     rsi, rcx               ; end of src
        add     rdi, rcx               ; end of dest
        neg     rcx                    ; negative index from the end
        cmp     ecx, -20H
        jg      A100        
        ; move 32 bytes
        ; mov is faster than movdqu on older processors (2008),
        ; movdqu may be faster on newer processors (2012)
     %define USEMOVDQU 1
     %if USEMOVDQU == 0
        mov     rax, [rsi+rcx]
        mov     rdx, [rsi+rcx+8]
        mov     [rdi+rcx], rax
        mov     [rdi+rcx+8], rdx
        mov     rax, qword [rsi+rcx+10H]
        mov     rdx, qword [rsi+rcx+18H]
        mov     qword [rdi+rcx+10H], rax
        mov     qword [rdi+rcx+18H], rdx
     %else
        movdqu  xmm0, [rsi+rcx]
        movdqu  xmm1, [rsi+rcx+10H]
        movdqu  [rdi+rcx], xmm0
        movdqu  [rdi+rcx+10H], xmm1     
     %endif   
        add     rcx, 20H
A100:   cmp     ecx, -10H        
        jg      A200
        ; move 16 bytes
     %if USEMOVDQU == 0
        mov     rax, [rsi+rcx]
        mov     rdx, [rsi+rcx+8]
        mov     [rdi+rcx], rax
        mov     [rdi+rcx+8], rdx
     %else
        movdqu  xmm0, [rsi+rcx]
        movdqu  [rdi+rcx], xmm0
     %endif   
        add     rcx, 10H
A200:   cmp     ecx, -8        
        jg      A300
        ; move 8 bytes
        mov     rax, qword [rsi+rcx]
        mov     qword [rdi+rcx], rax
        add     rcx, 8
A300:   cmp     ecx, -4        
        jg      A400
        ; move 4 bytes
        mov     eax, [rsi+rcx]
        mov     [rdi+rcx], eax
        add     rcx, 4
        jz      A900                     ; early out if count divisible by 4
A400:   cmp     ecx, -2        
        jg      A500
        ; move 2 bytes
        movzx   eax, word [rsi+rcx]
        mov     [rdi+rcx], ax
        add     rcx, 2
A500:   cmp     ecx, -1
        jg      A900        
        ; move 1 byte
        movzx   eax, byte [rsi+rcx]
        mov     [rdi+rcx], al
A900:   ; finished
        EPILOG        
        
B100:   ; count >= 64
        ; Note: this part will not always work if count < 64
        ; Calculate size of first block up to first regular boundary of dest
        mov     edx, edi
        neg     edx
        and     edx, 0FH
        jz      B300                    ; Skip if dest aligned by 16
        
        ; rdx = size of first partial block, 1 - 15 bytes
        add     rsi, rdx
        add     rdi, rdx
        sub     rcx, rdx
        neg     rdx
        cmp     edx, -8
        jg      B200
        ; move 8 bytes
        mov     rax, [rsi+rdx]
        mov     [rdi+rdx], rax
        add     rdx, 8
B200:   cmp     edx, -4        
        jg      B210
        ; move 4 bytes
        mov     eax, [rsi+rdx]
        mov     [rdi+rdx], eax
        add     rdx, 4
B210:   cmp     edx, -2        
        jg      B220
        ; move 2 bytes
        movzx   eax, word [rsi+rdx]
        mov     [rdi+rdx], ax
        add     rdx, 2
B220:   cmp     edx, -1
        jg      B300
        ; move 1 byte
        movzx   eax, byte [rsi+rdx]
        mov     [rdi+rdx], al
        
B300:   ; Now dest is aligned by 16. Any partial block has been moved        
        ; Find alignment of src modulo 16 at this point:
        mov     eax, esi
        and     eax, 0FH
        
        ; Set up for loop moving 32 bytes per iteration:
        mov     edx, ecx               ; Save count (lower 32 bits)
        and     rcx, -20H              ; Round down count to nearest multiple of 32
        add     rsi, rcx               ; Point to the end
        add     rdi, rcx               ; Point to the end
        sub     edx, ecx               ; Remaining data after loop (0-31)
        sub     rsi, rax               ; Nearest preceding aligned block of src

        ; Check if count very big
;        cmp     rcx, [CacheBypassLimit]
;        ja      B400                   ; Use non-temporal store if count > CacheBypassLimit
        neg     rcx                    ; Negative index from the end
        
        ; Dispatch to different codes depending on src alignment
;        lea     r8, [AlignmentDispatch]
        jmp     near [r8+rax*8]

B400:   neg     rcx
        ; Dispatch to different codes depending on src alignment
;        lea     r8, [AlignmentDispatchNT]
        jmp     near [r8+rax*8]
        

align   16
C100:   ; Code for aligned src.
        ; The nice case, src and dest have same alignment.

        ; Loop. rcx has negative index from the end, counting up to zero
        movaps  xmm0, [rsi+rcx]
        movaps  xmm1, [rsi+rcx+10H]
        movaps  [rdi+rcx], xmm0
        movaps  [rdi+rcx+10H], xmm1
        add     rcx, 20H
        jnz     C100
        
        ; Move the remaining edx bytes (0 - 31):
        add     rsi, rdx
        add     rdi, rdx
        neg     rdx
        jz      C500                   ; Skip if no more data
        ; move 16-8-4-2-1 bytes, aligned
        cmp     edx, -10H
        jg      C200
        ; move 16 bytes
        movaps  xmm0, [rsi+rdx]
        movaps  [rdi+rdx], xmm0
        add     rdx, 10H
C200:   cmp     edx, -8
        jg      C210        
        ; move 8 bytes
        mov     rax, [rsi+rdx]
        mov     [rdi+rdx], rax
        add     rdx, 8 
        jz      C500                   ; Early skip if count divisible by 8       
C210:   cmp     edx, -4
        jg      C220        
        ; move 4 bytes
        mov     eax, [rsi+rdx]
        mov     [rdi+rdx], eax
        add     rdx, 4        
C220:   cmp     edx, -2
        jg      C230        
        ; move 2 bytes
        movzx   eax, word [rsi+rdx]
        mov     [rdi+rdx], ax
        add     rdx, 2
C230:   cmp     edx, -1
        jg      C500        
        ; move 1 byte
        movzx   eax, byte [rsi+rdx]
        mov     [rdi+rdx], al
C500:   ; finished     
        EPILOG
        
       
; Code for each src alignment, SSE2 instruction set:
; Make separate code for each alignment u because the shift instructions
; have the shift count as a constant:

%MACRO  MOVE_UNALIGNED_SSE2  2 ; u, nt
; Move rcx + rdx bytes of data
; Source is misaligned. (src-dest) modulo 16 = %1
; %2 = 1 if non-temporal store desired
; eax = %1
; rsi = src - %1 = nearest preceding 16-bytes boundary
; rdi = dest (aligned)
; rcx = - (count rounded down to nearest divisible by 32)
; edx = remaining bytes to move after loop
        movdqa  xmm0, [rsi+rcx]        ; Read from nearest preceding 16B boundary
%%L1:  ; Loop. rcx has negative index from the end, counting up to zero
        movdqa  xmm1, [rsi+rcx+10H]    ; Read next two blocks aligned
        movdqa  xmm2, [rsi+rcx+20H]
        movdqa  xmm3, xmm1             ; Copy because used twice
        psrldq  xmm0, %1               ; shift right
        pslldq  xmm1, 16-%1            ; shift left
        por     xmm0, xmm1             ; combine blocks
        %IF %2 == 0
        movdqa  [rdi+rcx], xmm0        ; Save aligned
        %ELSE
        movntdq [rdi+rcx], xmm0        ; non-temporal save
        %ENDIF
        movdqa  xmm0, xmm2             ; Save for next iteration
        psrldq  xmm3, %1               ; shift right
        pslldq  xmm2, 16-%1            ; shift left
        por     xmm3, xmm2             ; combine blocks
        %IF %2 == 0
        movdqa  [rdi+rcx+10H], xmm3    ; Save aligned
        %ELSE
        movntdq [rdi+rcx+10H], xmm3    ; non-temporal save
        %ENDIF
        add     rcx, 20H               ; Loop through negative values up to zero
        jnz     %%L1
        
        ; Set up for edx remaining bytes
        add     rsi, rdx
        add     rdi, rdx
        neg     rdx
        cmp     edx, -10H
        jg      %%L2
        ; One more 16-bytes block to move
        movdqa  xmm1, [rsi+rdx+10H]
        psrldq  xmm0, %1               ; shift right
        pslldq  xmm1, 16-%1            ; shift left
        por     xmm0, xmm1             ; combine blocks
        %IF %2 == 0
        movdqa  [rdi+rdx], xmm0        ; Save aligned
        %ELSE
        movntdq [rdi+rdx], xmm0        ; non-temporal save
        %ENDIF        
        add     rdx, 10H        
%%L2:   ; Get src pointer back to misaligned state
        add     rsi, rax
        ; Move remaining 0 - 15 bytes, unaligned
        jmp     C200
%ENDMACRO


%MACRO  MOVE_UNALIGNED_SSE2_4  1 ; nt
; Special case for u = 4
; %1 = 1 if non-temporal store desired
        movaps  xmm0, [rsi+rcx]        ; Read from nearest preceding 16B boundary
%%L1:   ; Loop. rcx has negative index from the end, counting up to zero
        movaps  xmm1, [rsi+rcx+10H]    ; Read next two blocks aligned
        movss   xmm0, xmm1             ; Moves 4 bytes, leaves remaining bytes unchanged
        pshufd  xmm0, xmm0, 00111001B  ; Rotate
        %IF %1 == 0
        movdqa  [rdi+rcx], xmm0        ; Save aligned
        %ELSE
        movntdq [rdi+rcx], xmm0        ; Non-temporal save
        %ENDIF
        movaps  xmm0, [rsi+rcx+20H]
        movss   xmm1, xmm0
        pshufd  xmm1, xmm1, 00111001B
        %IF %1 == 0
        movdqa  [rdi+rcx+10H], xmm1    ; Save aligned
        %ELSE
        movntdq [rdi+rcx+10H], xmm1    ; Non-temporal save
        %ENDIF
        add     rcx, 20H               ; Loop through negative values up to zero
        jnz     %%L1        
        ; Set up for edx remaining bytes
        add     rsi, rdx
        add     rdi, rdx
        neg     rdx
        cmp     edx, -10H
        jg      %%L2
        ; One more 16-bytes block to move
        movaps  xmm1, [rsi+rdx+10H]    ; Read next two blocks aligned
        movss   xmm0, xmm1
        pshufd  xmm0, xmm0, 00111001B
        %IF %1 == 0
        movdqa  [rdi+rdx], xmm0        ; Save aligned
        %ELSE
        movntdq [rdi+rdx], xmm0        ; Non-temporal save
        %ENDIF
        add     rdx, 10H        
%%L2:   ; Get src pointer back to misaligned state
        add     rsi, rax
        ; Move remaining 0 - 15 bytes, unaligned
        jmp     C200
%ENDMACRO


%MACRO  MOVE_UNALIGNED_SSE2_8  1 ; nt
; Special case for u = 8
; %1 = 1 if non-temporal store desired
        movaps  xmm0, [rsi+rcx]        ; Read from nearest preceding 16B boundary
%%L1:   ; Loop. rcx has negative index from the end, counting up to zero
        movaps  xmm1, [rsi+rcx+10H]    ; Read next two blocks aligned
        movsd   xmm0, xmm1             ; Moves 8 bytes, leaves remaining bytes unchanged
        shufps  xmm0, xmm0, 01001110B  ; Rotate
        %IF %1 == 0
        movdqa  [rdi+rcx], xmm0        ; Save aligned
        %ELSE
        movntdq [rdi+rcx], xmm0        ; Non-temporal save
        %ENDIF
        movaps  xmm0, [rsi+rcx+20H]
        movsd   xmm1, xmm0
        shufps  xmm1, xmm1, 01001110B
        %IF %1 == 0
        movdqa  [rdi+rcx+10H], xmm1    ; Save aligned
        %ELSE
        movntdq [rdi+rcx+10H], xmm1    ; Non-temporal save
        %ENDIF
        add     rcx, 20H               ; Loop through negative values up to zero
        jnz     %%L1        
        ; Set up for edx remaining bytes
        add     rsi, rdx
        add     rdi, rdx
        neg     rdx
        cmp     edx, -10H
        jg      %%L2
        ; One more 16-bytes block to move
        movaps  xmm1, [rsi+rdx+10H]    ; Read next two blocks aligned
        movsd   xmm0, xmm1
        shufps  xmm0, xmm0, 01001110B
        %IF %1 == 0
        movdqa  [rdi+rdx], xmm0        ; Save aligned
        %ELSE
        movntdq [rdi+rdx], xmm0        ; Non-temporal save
        %ENDIF
        add     rdx, 10H        
%%L2:   ; Get src pointer back to misaligned state
        add     rsi, rax
        ; Move remaining 0 - 15 bytes, unaligned
        jmp     C200
%ENDMACRO


%MACRO  MOVE_UNALIGNED_SSE2_12  1 ; nt
; Special case for u = 12
; %1 = 1 if non-temporal store desired
        movaps  xmm0, [rsi+rcx]        ; Read from nearest preceding 16B boundary
        pshufd  xmm0, xmm0, 10010011B
%%L1:   ; Loop. rcx has negative index from the end, counting up to zero
        movaps  xmm1, [rsi+rcx+10H]    ; Read next two blocks aligned
        movaps  xmm2, [rsi+rcx+20H]
        pshufd  xmm1, xmm1, 10010011B
        pshufd  xmm2, xmm2, 10010011B
        movaps  xmm3, xmm2
        movss   xmm2, xmm1             ; Moves 4 bytes, leaves remaining bytes unchanged
        movss   xmm1, xmm0             ; Moves 4 bytes, leaves remaining bytes unchanged       
        %IF %1 == 0
        movdqa  [rdi+rcx], xmm1        ; Save aligned
        movdqa  [rdi+rcx+10H], xmm2    ; Save aligned
        %ELSE
        movntdq [rdi+rcx], xmm1        ; Non-temporal save
        movntdq [rdi+rcx+10H], xmm2    ; Non-temporal save
        %ENDIF
        movaps  xmm0, xmm3             ; Save for next iteration        
        add     rcx, 20H               ; Loop through negative values up to zero
        jnz     %%L1        
        ; Set up for edx remaining bytes
        add     rsi, rdx
        add     rdi, rdx
        neg     rdx
        cmp     edx, -10H
        jg      %%L2
        ; One more 16-bytes block to move
        movaps  xmm1, [rsi+rdx+10H]    ; Read next two blocks aligned
        pshufd  xmm1, xmm1, 10010011B
        movss   xmm1, xmm0             ; Moves 4 bytes, leaves remaining bytes unchanged       
        %IF %1 == 0
        movdqa  [rdi+rdx], xmm1        ; Save aligned
        %ELSE
        movntdq [rdi+rdx], xmm1        ; Non-temporal save
        %ENDIF
        add     rdx, 10H        
%%L2:   ; Get src pointer back to misaligned state
        add     rsi, rax
        ; Move remaining 0 - 15 bytes, unaligned
        jmp     C200
%ENDMACRO


; Code for each src alignment, Suppl.SSE3 instruction set:
; Make separate code for each alignment u because the palignr instruction
; has the shift count as a constant:

%MACRO MOVE_UNALIGNED_SSSE3  1 ; u
; Move rcx + rdx bytes of data
; Source is misaligned. (src-dest) modulo 16 = %1
; eax = %1
; rsi = src - %1 = nearest preceding 16-bytes boundary
; rdi = dest (aligned)
; rcx = - (count rounded down to nearest divisible by 32)
; edx = remaining bytes to move after loop
        movdqa  xmm0, [rsi+rcx]        ; Read from nearest preceding 16B boundary
        
%%L1:   ; Loop. rcx has negative index from the end, counting up to zero
        movdqa  xmm2, [rsi+rcx+10H]    ; Read next two blocks
        movdqa  xmm3, [rsi+rcx+20H]
        movdqa  xmm1, xmm0             ; Save xmm0
        movdqa  xmm0, xmm3             ; Save for next iteration
        palignr xmm3, xmm2, %1         ; Combine parts into aligned block
        palignr xmm2, xmm1, %1         ; Combine parts into aligned block
        movdqa  [rdi+rcx], xmm2        ; Save aligned
        movdqa  [rdi+rcx+10H], xmm3    ; Save aligned
        add     rcx, 20H
        jnz     %%L1
        
        ; Set up for edx remaining bytes
        add     rsi, rdx
        add     rdi, rdx
        neg     rdx
        cmp     edx, -10H
        jg      %%L2
        ; One more 16-bytes block to move
        movdqa  xmm2, [rsi+rdx+10H]
        palignr xmm2, xmm0, %1
        movdqa  [rdi+rdx], xmm2
        add     rdx, 10H        
%%L2:   ; Get src pointer back to misaligned state
        add     rsi, rax
        ; Move remaining 0 - 15 bytes
        jmp     C200
%ENDMACRO


; Code for each src alignment, AVX instruction set:
; Make separate code for each alignment u because the vpalignr instruction
; has the shift count as a constant:

%MACRO MOVE_UNALIGNED_AVXA  1 ; u
; Move rcx + rdx bytes of data
; Source is misaligned. (src-dest) modulo 16 = %1
; eax = %1
; rsi = src - %1 = nearest preceding 16-bytes boundary
; rdi = dest (aligned)
; rcx = - (count rounded down to nearest divisible by 32)
; edx = remaining bytes to move after loop
        vmovdqa  xmm0, [rsi+rcx]       ; Read from nearest preceding 16B boundary
        
%%L1:   ; Loop. rcx has negative index from the end, counting up to zero
        vmovdqa  xmm2, [rsi+rcx+10H]   ; Read next two blocks
        vmovdqa  xmm3, [rsi+rcx+20H]
        vpalignr xmm4, xmm2, xmm0, %1  ; Combine parts into aligned block
        vpalignr xmm5, xmm3, xmm2, %1  ; Combine parts into aligned block
        vmovdqa  [rdi+rcx], xmm4       ; Save aligned
        vmovdqa  [rdi+rcx+10H], xmm5   ; Save aligned
        vmovdqa  xmm0, xmm3            ; Save for next iteration        
        add     rcx, 20H
        jnz     %%L1
        
        ; Set up for edx remaining bytes
        add     rsi, rdx
        add     rdi, rdx
        neg     rdx
        cmp     edx, -10H
        jg      %%L2
        ; One more 16-bytes block to move
        vmovdqa  xmm2, [rsi+rdx+10H]
        vpalignr xmm4, xmm2, xmm0, %1
        vmovdqa  [rdi+rdx], xmm4
        add     rdx, 10H        
%%L2:   ; Get src pointer back to misaligned state
        add     rsi, rax
        ; Move remaining 0 - 15 bytes
        jmp     C200
%ENDMACRO


; Make 15 instances of SSE2 macro for each value of the alignment u.
; These are pointed to by the jump table AlignmentDispatchSSE2 below
; (alignments are inserted manually to minimize the number of 16-bytes
;  boundaries inside loops)

D101:   MOVE_UNALIGNED_SSE2 1,   0
D102:   MOVE_UNALIGNED_SSE2 2,   0
D103:   MOVE_UNALIGNED_SSE2 3,   0
D104:   MOVE_UNALIGNED_SSE2_4    0
D105:   MOVE_UNALIGNED_SSE2 5,   0
D106:   MOVE_UNALIGNED_SSE2 6,   0
D107:   MOVE_UNALIGNED_SSE2 7,   0
align   4
D108:   MOVE_UNALIGNED_SSE2_8    0
D109:   MOVE_UNALIGNED_SSE2 9,   0
D10A:   MOVE_UNALIGNED_SSE2 0AH, 0
D10B:   MOVE_UNALIGNED_SSE2 0BH, 0
D10C:   MOVE_UNALIGNED_SSE2_12   0
align   8
D10D:   MOVE_UNALIGNED_SSE2 0DH, 0
D10E:   MOVE_UNALIGNED_SSE2 0EH, 0
D10F:   MOVE_UNALIGNED_SSE2 0FH, 0
        
; Make 15 instances of Suppl-SSE3 macro for each value of the alignment u.
; These are pointed to by the jump table AlignmentDispatchSupSSE3 below

align   8
E101:   MOVE_UNALIGNED_SSSE3 1
E102:   MOVE_UNALIGNED_SSSE3 2
E103:   MOVE_UNALIGNED_SSSE3 3
E104:   MOVE_UNALIGNED_SSSE3 4
E105:   MOVE_UNALIGNED_SSSE3 5
E106:   MOVE_UNALIGNED_SSSE3 6
E107:   MOVE_UNALIGNED_SSSE3 7
E108:   MOVE_UNALIGNED_SSSE3 8
E109:   MOVE_UNALIGNED_SSSE3 9
E10A:   MOVE_UNALIGNED_SSSE3 0AH
E10B:   MOVE_UNALIGNED_SSSE3 0BH
align 8
E10C:   MOVE_UNALIGNED_SSSE3 0CH
E10D:   MOVE_UNALIGNED_SSSE3 0DH
E10E:   MOVE_UNALIGNED_SSSE3 0EH
E10F:   MOVE_UNALIGNED_SSSE3 0FH

; Codes for non-temporal move. Aligned case first

F100:   ; Non-temporal move, src and dest have same alignment.
        ; Loop. rcx has negative index from the end, counting up to zero
        movaps  xmm0, [rsi+rcx]        ; Read
        movaps  xmm1, [rsi+rcx+10H]
        movntps [rdi+rcx], xmm0        ; Write non-temporal (bypass cache)
        movntps [rdi+rcx+10H], xmm1
        add     rcx, 20H
        jnz     F100                   ; Loop through negative rcx up to zero
                
        ; Move the remaining edx bytes (0 - 31):
        add     rsi, rdx
        add     rdi, rdx
        neg     rdx
        jz      C500                   ; Skip if no more data
        ; Check if we can more one more 16-bytes block
        cmp     edx, -10H
        jg      C200
        ; move 16 bytes, aligned
        movaps  xmm0, [rsi+rdx]
        movntps [rdi+rdx], xmm0
        add     rdx, 10H
        ; move the remaining 0 - 15 bytes
        jmp     C200

; Make 15 instances of MOVE_UNALIGNED_SSE2 macro for each value of 
; the alignment u.
; These are pointed to by the jump table AlignmentDispatchNT below

F101:   MOVE_UNALIGNED_SSE2 1,   1
F102:   MOVE_UNALIGNED_SSE2 2,   1
F103:   MOVE_UNALIGNED_SSE2 3,   1
F104:   MOVE_UNALIGNED_SSE2_4    1
F105:   MOVE_UNALIGNED_SSE2 5,   1
F106:   MOVE_UNALIGNED_SSE2 6,   1
F107:   MOVE_UNALIGNED_SSE2 7,   1
F108:   MOVE_UNALIGNED_SSE2_8    1
F109:   MOVE_UNALIGNED_SSE2 9,   1
F10A:   MOVE_UNALIGNED_SSE2 0AH, 1
F10B:   MOVE_UNALIGNED_SSE2 0BH, 1
F10C:   MOVE_UNALIGNED_SSE2_12   1
F10D:   MOVE_UNALIGNED_SSE2 0DH, 1
F10E:   MOVE_UNALIGNED_SSE2 0EH, 1
F10F:   MOVE_UNALIGNED_SSE2 0FH, 1

; Make 15 instances of MOVE_UNALIGNED_AVXA macro for each value of the alignment u.
; These are pointed to by the jump table AlignmentDispatchAVXA below

align   8
G101:   MOVE_UNALIGNED_AVXA 1
G102:   MOVE_UNALIGNED_AVXA 2
G103:   MOVE_UNALIGNED_AVXA 3
G104:   MOVE_UNALIGNED_AVXA 4
G105:   MOVE_UNALIGNED_AVXA 5
G106:   MOVE_UNALIGNED_AVXA 6
G107:   MOVE_UNALIGNED_AVXA 7
G108:   MOVE_UNALIGNED_AVXA 8
G109:   MOVE_UNALIGNED_AVXA 9
G10A:   MOVE_UNALIGNED_AVXA 0AH
G10B:   MOVE_UNALIGNED_AVXA 0BH
G10C:   MOVE_UNALIGNED_AVXA 0CH
G10D:   MOVE_UNALIGNED_AVXA 0DH
G10E:   MOVE_UNALIGNED_AVXA 0EH
G10F:   MOVE_UNALIGNED_AVXA 0FH   


align 16

; Use AMD XOP instruction VPPERM to shift and align source without the
; clumsy dispatch lists
MEMCPYXOP:
        PROLOG
        mov     rcx, rdx
        mov     r9,  rdi               ; dest
        cmp     rcx, 40H
        jb      B80                    ; Use simpler code if count < 64
       
        ; count >= 64
        ; Note: this part will not always work if count < 64
        ; Calculate size of first block up to first regular boundary of dest
        mov     edx, edi
        neg     edx
        and     edx, 0FH
        jz      X300                    ; Skip if dest aligned by 16
        
        ; rdx = size of first partial block, 1 - 15 bytes
        add     rsi, rdx
        add     rdi, rdx
        sub     rcx, rdx
        neg     rdx
        cmp     edx, -8
        jg      X200
        ; move 8 bytes
        mov     rax, [rsi+rdx]
        mov     [rdi+rdx], rax
        add     rdx, 8
X200:   cmp     edx, -4        
        jg      X210
        ; move 4 bytes
        mov     eax, [rsi+rdx]
        mov     [rdi+rdx], eax
        add     rdx, 4
X210:   cmp     edx, -2        
        jg      X220
        ; move 2 bytes
        movzx   eax, word [rsi+rdx]
        mov     [rdi+rdx], ax
        add     rdx, 2
X220:   cmp     edx, -1
        jg      X300
        ; move 1 byte
        movzx   eax, byte [rsi+rdx]
        mov     [rdi+rdx], al
        
X300:   ; Now dest is aligned by 16. Any partial block has been moved        
        ; Find alignment of src modulo 16 at this point:
        mov     eax, esi
        and     eax, 0FH
        
        ; Set up for loop moving 32 bytes per iteration:
        mov     edx, ecx               ; Save count (lower 32 bits)
        and     rcx, -20H              ; Round down count to nearest multiple of 32
        add     rsi, rcx               ; Point to the end
        add     rdi, rcx               ; Point to the end
        sub     edx, ecx               ; Remaining data after loop (0-31)
        sub     rsi, rax               ; Nearest preceding aligned block of src

        ; Check if count very big
;        cmp     rcx, [CacheBypassLimit]
;        ja      B400                   ; Use non-temporal store if count > CacheBypassLimit

        neg     rcx                    ; Negative index from the end

; Code for unaligned src, AMD XOP instruction set
; Move ecx + edx bytes of data
; Source is misaligned. (src-dest) modulo 16 = u
; eax = u
; esi = src - u = nearest preceding 16-bytes boundary
; edi = dest (aligned)
; ecx = - (count rounded down to nearest divisible by 32)
; edx = remaining bytes to move after loop

        lea     r8, [rel PermMask]
        movdqu  xmm5, [r8+rax]            ; Mask for permutation by u
        movdqa  xmm1, [rsi+rcx]           ; Read from nearest preceding 16B boundary
        
X400:   ; Loop. rcx has negative index from the end, counting up to zero
        movdqa  xmm2, [rsi+rcx+10H]       ; Read next block aligned
        vpperm  xmm3, xmm1, xmm2, xmm5    ; Combine bytes from xmm1 and xmm2
        movdqa  xmm1, [rsi+rcx+20H]       ; Read next block aligned
        vpperm  xmm4, xmm2, xmm1, xmm5    ; Combine bytes from xmm2 and xmm1
        movdqa  [rdi+rcx], xmm3           ; Save aligned
        movdqa  [rdi+rcx+10H], xmm4       ; Save aligned
        add     rcx, 20H                  ; Loop through negative values up to zero
        jnz     X400
        
        ; Set up for edx remaining bytes
        add     rsi, rdx
        add     rdi, rdx
        neg     rdx
        cmp     edx, -10H
        jg      X500
        ; One more 16-bytes block to move
        movdqa  xmm2, [rsi+rdx+10H]       ; Read next block aligned
        vpperm  xmm3, xmm1, xmm2, xmm5    ; Combine bytes from xmm1 and xmm2
        movdqa  [rdi+rdx], xmm3           ; Save aligned
        add     rdx, 10H
        
X500:   ; Get src pointer back to misaligned state
        add     rsi, rax
        ; Move remaining 0 - 15 bytes, unaligned

        cmp     edx, -8
        jg      X610        
        ; move 8 bytes
        mov     rax, [rsi+rdx]
        mov     [rdi+rdx], rax
        add     rdx, 8 
        jz      X700                   ; Early skip if count divisible by 8       
X610:   cmp     edx, -4
        jg      X620        
        ; move 4 bytes
        mov     eax, [rsi+rdx]
        mov     [rdi+rdx], eax
        add     rdx, 4        
X620:   cmp     edx, -2
        jg      X630        
        ; move 2 bytes
        movzx   eax, word [rsi+rdx]
        mov     [rdi+rdx], ax
        add     rdx, 2
X630:   cmp     edx, -1
        jg      X700        
        ; move 1 byte
        movzx   eax, byte [rsi+rdx]
        mov     [rdi+rdx], al
X700:   ; finished     
        EPILOG


; MEMCPYAVX512F:
; code for small counts. (This is slower than older versions if branches are predicted)
align 16
H000:   xor     r8d, r8d
        mov     eax, -1
        mov     ecx, edx
        shr     edx, 2
H010:   bzhi    eax, eax, edx                    ; set mask k1 to move edx 32-bit words, at most 16
        kmovw   k1, eax
        vmovdqu32 zmm0{k1}{z}, [rsi+r8]
        vmovdqu32 [rdi+r8]{k1}, zmm0
        add     r8d, 40H
        sub     edx, 10H
        jg      H010
        ; now there are 0-3 bytes remaining
        mov     edx, ecx
        and     edx, -4                          ; round down to get number of bytes moved so far
        test    ecx, 2
        jz      H020
        mov     ax, [rsi+rdx]                    ; move 2 bytes
        mov     [rdi+rdx], ax
        add     edx, 2
H020:   test    ecx, 1
        jz      H030
        mov     al, [rsi+rdx]                    ; move 1 byte
        mov     [rdi+rdx], al
H030:   vzeroupper                               ; might do more harm than good on Knights Landing
        EPILOG
        

; Use AVX512F instructions to move 64 bytes at a time. Use mask with 4 bytes granularity
align 16        
MEMCPYAVX512F:
        PROLOG
; rdi = dest
; rsi = src
; rdx = count
        mov     r9,  rdi
;        cmp     rdx, 100H
        cmp     rdx, 80H
        jbe     H000                             ; Small version. Don't align
        mov     ecx, 40H
        sub     rdx, rcx                         ; Number of bytes to move minus 40H
        add     rdi, rdx                         ; Point to 40H bytes before end of source and destination
        add     rsi, rdx
        neg     rdx
        mov     eax, r9d
        and     eax, 3FH                         ; Align destination by 40H
        jz      H180
        sub     ecx, eax                         ; Number of bytes to move to align destination
        mov     eax, ecx
        shr     ecx, 2
        mov     r8d, -1
        bzhi    r8d, r8d, ecx                    ; set mask k1 to move ecx 32-bit words, at most 16
        kmovw   k1, r8d
        vmovdqu32 zmm0{k1}{z}, [rsi+rdx]
        vmovdqu32 [rdi+rdx]{k1}, zmm0
        mov     ecx, eax
        and     eax, -4                          ; number of bytes moved so far
        add     rdx, rax                         ; update index
        test    ecx, 2
        jz      H100
        mov     ax, [rsi+rdx]                    ; move 2 bytes
        mov     [rdi+rdx], ax
        add     rdx, 2
H100:   test    ecx, 1
        jz      H180
        mov     al, [rsi+rdx]                    ; move 1 byte
        mov     [rdi+rdx], al
        add     rdx, 1
H180:   ; now destination is aligned by 40H
        ; rsi = 40H before end of source
        ; rdi = 40H before end of destination
        ; rdx   = -(number of bytes remaining - 40H)

; align ?
H200:   ; main loop. Move 40H bytes at a time
        vmovdqu64 zmm0, [rsi+rdx]
        vmovdqa64 [rdi+rdx], zmm0
        add     rdx, 40H
        jle     H200

        ; remaining number of bytes to move = 40H - rdx
        mov     ecx, 40H
        sub     ecx, edx
        jz      H300
        mov     eax, ecx
        shr     ecx, 2
        mov     r8d, -1
        bzhi    r8d, r8d, ecx                    ; set mask k1 to move ecx 32-bit words, at most 16
        kmovw   k1, r8d
        vmovdqu32 zmm0{k1}{z}, [rsi+rdx]
        vmovdqa32 [rdi+rdx]{k1}, zmm0
        and     eax, -4
        add     edx, eax                         ; edx = 40H-remaining = 3DH .. 40H
        cmp     edx, 3FH
        jnb     H210
        mov     ax, [rsi+rdx]                    ; move 2 bytes
        mov     [rdi+rdx], ax
        add     edx, 2
H210:   test    edx, 1
        jz      H220
        mov     al, [rsi+rdx]                    ; move 1 byte
        mov     [rdi+rdx], al
H220:
H300:   vzeroupper
        EPILOG



; MEMCPYAVX512BW:
align 16
I000:   xor     ecx, ecx
        or      rax, -1                          ; mov rax,-1 instruction is 10 bytes long, this is shorter
I010:   bzhi    rax, rax, rdx                    ; set mask k1 to move rdx bytes, at most 40H
        kmovq   k1, rax
        vmovdqu8 zmm0{k1}{z}, [rsi+rcx]
        vmovdqu8 [rdi+rcx]{k1}, zmm0
        add     ecx, 40H
        sub     edx, 40H
        jg      I010
        vzeroupper
        EPILOG
        
; Use AVX512BW instructions to move 64 bytes at a time. Use mask with 1 byte granularity
align 16        
MEMCPYAVX512BW:
        PROLOG
; rdi = dest
; rsi = src
; rdx = count
        mov     r9,  rdi
        cmp     rdx, 100H
        jb      I000                             ; Less than 4x vector size. Don't align
        mov     ecx, 40H
        sub     rdx, rcx                         ; Number of bytes to move minus 40H
        add     rdi, rdx                         ; Point to 40H bytes before end of source and destination
        add     rsi, rdx
        neg     rdx
        mov     eax, r9d
        and     eax, 3FH                         ; Align destination by 40H
;       jz      I100                             ; optional shortcut. Saves little but costs a possible branch misprediction
        sub     ecx, eax                         ; Number of bytes to move to align destination
        or      r8, -1
        bzhi    r8, r8, rcx                      ; set mask k1 to move ecx bytes, at most 64
        kmovq   k1, r8
        vmovdqu8 zmm0{k1}{z}, [rsi+rdx]
        vmovdqu8 [rdi+rdx]{k1}, zmm0
        add     rdx, rcx
I100:   ; now destination is aligned by 40H
        ; rsi = 40H before end of source
        ; rdi = 40H before end of destination
        ; rdx   = -(number of bytes remaining - 40H)
        
; align ?
I200:   ; main loop. Move 40H bytes at a time
        vmovdqu64 zmm0, [rsi+rdx]
        vmovdqa64 [rdi+rdx], zmm0
        add     rdx, 40H
        jle     I200

        ; remaining number of bytes to move = 40H - rdx
        mov     ecx, 40H
        sub     ecx, edx
;       jz      I300                             ; optional shortcut. Saves little but costs a possible branch misprediction
        or      rax, -1
        bzhi    rax, rax, rcx                    ; set mask k1 to move rcx bytes
        kmovq   k1, rax
        vmovdqu8 zmm0{k1}{z}, [rsi+rdx]
        vmovdqu8 [rdi+rdx]{k1}, zmm0
I300:   vzeroupper
        EPILOG
        

        
Q100:   ; CPU dispatcher, check for instruction set
        ; This part is executed only once, optimized for size
        push    rax
        push    rbx
        push    rcx
        push    rdx
        push    rsi
        push    rdi
        mov     eax, 1
        cpuid                          ; Get feature flags
        lea     rsi, [AlignmentDispatchSSE2]
        bt      ecx, 9                 ; Test bit for SupSSE3
        jnc     Q200
        lea     rsi, [AlignmentDispatchSupSSE3]
Q200:   ; Insert appropriate table
        mov     rdi, r8
        mov     ecx, 16
        rep     movsq
        pop     rdi
        pop     rsi
        pop     rdx
        pop     rcx
        pop     rbx
        pop     rax
        ; Jump according to the replaced table
        jmp     near [r8+rax*8]


; Data segment must be included in function namespace
SECTION .data  align=16
default rel

; Jump tables for alignments 0 - 15:
; The CPU dispatcher replaces AlignmentDispatch with 
; AlignmentDispatchSSE2 or AlignmentDispatchSupSSE3 if Suppl-SSE3 
; is supported.

AlignmentDispatch:
DQ Q100, Q100, Q100, Q100, Q100, Q100, Q100, Q100
DQ Q100, Q100, Q100, Q100, Q100, Q100, Q100, Q100

; Code pointer for each alignment for SSE2 instruction set
AlignmentDispatchSSE2:
DQ C100, D101, D102, D103, D104, D105, D106, D107
DQ D108, D109, D10A, D10B, D10C, D10D, D10E, D10F

; Code pointer for each alignment for Suppl-SSE3 instruction set
AlignmentDispatchSupSSE3:
DQ C100, E101, E102, E103, E104, E105, E106, E107
DQ E108, E109, E10A, E10B, E10C, E10D, E10E, E10F

; Code pointer for each alignment for non-temporal store
AlignmentDispatchNT:
DQ F100, F101, F102, F103, F104, F105, F106, F107
DQ F108, F109, F10A, F10B, F10C, F10D, F10E, F10F

; Code pointer for each alignment for AVX instruction set (16 bytes moves)
AlignmentDispatchAVXA:
DQ C100, G101, G102, G103, G104, G105, G106, G107
DQ G108, G109, G10A, G10B, G10C, G10D, G10E, G10F

; Permutation mask used by XOP version
PermMask DB  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15
         DB 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31

; Bypass cache by using non-temporal moves if count > CacheBypassLimit
; The optimal value of _CacheBypassLimit is difficult to estimate, but
; a reasonable value is half the size of the largest cache:
CacheBypassLimit: DQ 400000H              ; 400000H = 4 Megabytes


