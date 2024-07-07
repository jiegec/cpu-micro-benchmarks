;----------------------------------------------------------------------------
;        mixed_throughput.inc                          2021-03-15 Agner Fog
;
; PMC Test program for testing the throughput for mixed instructions
;
; Outputs number of instructions per clock and number of bytes per clock
; NOTE: nbytes has not been checked. Numbers may be wrong
;
; Constants to be defined:
;
; repeatx1:       Repeat count for loop around testcode
;
; repeatx2:       Repeat count for repeat macro around testcode
;
;
; tcase:     Test case:
;            1: 1-byte NOPs
;            2: 12-byte NOPs
;            3: integer 2 mov, 2 add, 2 and
;            4: integer 2 read, 1 write
;            5: integer 1 read, 2 write
;            6: integer 1 read, 1 write, 4 add
;            7: integer read, write, 2 mov, and, cmp/jump not taken
;
;           10: vector 2 read, 128 bits
;           11: vector 1 read, 1 write, 1 mov, 128 bits
;           12: vector 2 read, 1 write, 1 mov, 128 bits
;
;           20: vector 2 read, 1 write, 1 padd, 256 bits
;           21: vector 1 read, 2 write, 1 padd, 256 bits
;           22: vector 1 read, 1 write, 2 padd, 1 pmul, 256 bits
;           23: vector 1 read, 1 write, 2 padd, 2 pmul, 256 bits
;           24: integer 2 read, 1 write, vector 1 read, 256 bits
;           25: integer 1 read, 1 write, vector 1 read, 256 bits
;           26: integer 1 read, 1 write, vector 1 write, 256 bits
;           27: integer 1 read, vector 1 read, 1 write, 256 bits
;           28: integer 2 read, vector 1 read, 256 bits
;           29: integer 2 read, vector 1 write, 256 bits
;           30: integer 1 read, vector 2 read, 256 bits
;
;           40: integer 1 read, 1 write, vector 1 read, 1 write, 512 bits
;
;           60: 2 float add, 2 float mul, 256 bits
;           61: 2 float add, 2 fma, 256 bits
;
; (c) Copyright 2021 by Agner Fog. GNU General Public License www.gnu.org/licenses
;-----------------------------------------------------------------------------
; Define any undefined macros

; define nops of all lengths

%ifndef noptype
   %define noptype 2
%endif

; loops here
%ifndef repeatx1 
   %define repeatx1 100
%endif

%ifndef repeatx2 
   %define repeatx2 100
%endif

; disable default loops in templateB64.nasm
%define repeat1 1
%define repeat2 1


%include "nops.inc"


; Define each test case

%if tcase == 1  ; 1-byte NOPs

   %macro testcode 0
      mov ecx, repeatx1
      align 16
      %%LL1:
      %rep repeatx2 * 4
         NOP
      %endrep
      dec ecx
      jnz %%LL1
   %endmacro

   %define nbytes 4

%elif tcase == 2   ; 12-byte NOPs

   %macro testcode 0
      mov ecx, repeatx1
      align 16
      %%LL1:
      %rep repeatx2
         nop12
      %endrep
      dec ecx
      jnz %%LL1
   %endmacro

   %define nbytes 12

%elif tcase == 3   ; integer 2 mov, 2 add, 2 and

   %macro testcode 0
      mov r8d, repeatx1
      align 16
      %%LL1:
         %rep repeatx2
            mov eax, ebx
            add ecx, ebx
            and edx, ebx
            mov esi, ebx
            add edi, ebx
            and ebp, ebx
         %endrep
      dec r8d
      jnz %%LL1
   %endmacro

   %define nbytes 12

%elif tcase == 4   ; integer 2 read, 1 write

   %macro testcode 0
      mov ecx, repeatx1
      align 16
      %%LL1:
         %rep repeatx2
            mov eax, [psi]
            mov [psi+8], ebx
            mov edx, [psi+16]
         %endrep
      dec ecx
      jnz %%LL1
   %endmacro

   %define nbytes 8

%elif tcase == 5   ; integer 1 read, 2 write

   %macro testcode 0
      mov ecx, repeatx1
      align 16
      %%LL1:
         %rep repeatx2
            mov eax, [psi]
            mov [psi+8], ebx
            mov [psi+16], edx
         %endrep
      dec ecx
      jnz %%LL1
   %endmacro

   %define nbytes 8
   
%elif tcase == 6   ; integer 1 read, 1 write, 4 add

   %macro testcode 0
      mov r8d, repeatx1
      align 16
      %%LL1:
         %rep repeatx2
            mov eax, [psi]
            mov [psi+8], ebx
            add ecx, ebx
            add edx, ebx
            add edi, 1
            add ebp, 1
         %endrep
      dec r8d
      jnz %%LL1
   %endmacro

   %define nbytes 15


%elif tcase == 7   ; integer read, write, 2 mov, and, cmp/jump not taken

   %macro testcode 0
      mov r8d, repeatx1
      align 16
      %%LL1:
         %rep repeatx2
            mov eax, [psi]
            mov [psi+8], ebx
            mov ecx, ebx
            mov edx, ebx
            and edi, 1
            cmp ebp, ebp
            jne $+2
         %endrep
      dec r8d
      jnz %%LL1
   %endmacro

   %define nbytes 16

%elif tcase == 10   ; vector 2 read, 128 bits;

   %macro testcode 0
      mov r8d, repeatx1
      align 16
      %%LL1:
         %rep repeatx2
            movdqa xmm1, [psi]
            movdqa xmm2, [psi+32]
         %endrep
      dec r8d
      jnz %%LL1
   %endmacro

   %define nbytes 9

%elif tcase == 11   ; vector 1 read, 1 write, 1 mov, 128 bits;

   %macro testcode 0
      mov r8d, repeatx1
      align 16
      %%LL1:
         %rep repeatx2
            movdqa xmm1, [psi]
            movdqa [psi+32], xmm2
            movdqa xmm3, xmm4
         %endrep
      dec r8d
      jnz %%LL1
   %endmacro

   %define nbytes 13

%elif tcase == 12   ; vector 2 read, 1 write, 1 mov, 128 bits;

   %macro testcode 0
      mov r8d, repeatx1
      align 16
      %%LL1:
         %rep repeatx2
            movdqa xmm1, [psi]
            movdqa xmm2, [psi+16]
            movdqa [psi+32], xmm3
            movdqa xmm4, xmm5
         %endrep
      dec r8d
      jnz %%LL1
   %endmacro

   %define nbytes 18


%elif tcase == 20   ; vector 2 read, 1 write, 1 padd, 256 bits

   %macro testcode 0
      mov r8d, repeatx1
      align 16
      %%LL1:
         %rep repeatx2
            vmovdqa ymm1, [psi]
            vmovdqa ymm2, [psi+32]
            vmovdqa [psi+64], ymm3
            vpaddd ymm4, ymm5, ymm5
         %endrep
      dec r8d
      jnz %%LL1
   %endmacro

   %define nbytes 18

   
%elif tcase == 21   ; vector 1 read, 2 write, 1 padd

   %macro testcode 0
      mov r8d, repeatx1
      align 16
      %%LL1:
         %rep repeatx2
            vmovdqa ymm1, [psi]
            vmovdqa [psi+32], ymm2
            vmovdqa [psi+64], ymm3
            vpaddd ymm4, ymm5, ymm5
         %endrep
      dec r8d
      jnz %%LL1
   %endmacro

   %define nbytes 18 

%elif tcase == 22   ;  vector 1 read, 1 write, 2 padd, 1 pmul, 256 bits

   %macro testcode 0
      mov r8d, repeatx1
      align 16
      %%LL1:
         %rep repeatx2
            vmovdqa ymm1, [psi]
            vmovdqa [psi+32], ymm2
            vpaddd ymm3, ymm5, ymm5
            vpaddd ymm4, ymm5, ymm6
            vpmullw ymm7, ymm6, ymm2
         %endrep
      dec r8d
      jnz %%LL1
   %endmacro

   %define nbytes 21 

%elif tcase == 23   ;  vector 1 read, 1 write, 2 padd, 2 pmul, 256 bits

   %macro testcode 0
      mov r8d, repeatx1
      align 16
      %%LL1:
         %rep repeatx2
            vmovdqa ymm1, [psi]
            vmovdqa [psi+32], ymm2
            vpaddd ymm3, ymm5, ymm5
            vpaddd ymm4, ymm5, ymm6
            vpmullw ymm6, ymm5, ymm0
            vpmullw ymm7, ymm5, ymm2
         %endrep
      dec r8d
      jnz %%LL1
   %endmacro

   %define nbytes 25 

%elif tcase == 24   ; integer 2 read, 1 write, vector 1 read

   %macro testcode 0
      mov r8d, repeatx1
      align 16
      %%LL1:
         %rep repeatx2
            mov eax, [psi]
            mov ebx, [psi+8]
            mov [psi+16], ecx
            vmovdqa ymm1, [psi+32]
         %endrep
      dec r8d
      jnz %%LL1
   %endmacro

   %define nbytes 13

   
%elif tcase == 25   ; integer 1 read, 1 write, vector 1 read, 256 bits

   %macro testcode 0
      mov r8d, repeatx1
      align 16
      %%LL1:
         %rep repeatx2
            mov eax, [psi]
            mov [psi+8], ebx
            vmovdqa ymm1, [psi+32]
         %endrep
      dec r8d
      jnz %%LL1
   %endmacro

   %define nbytes 13


%elif tcase == 26   ; integer 1 read, 1 write, vector 1 write, 256 bits

   %macro testcode 0
      mov r8d, repeatx1
      align 16
      %%LL1:
         %rep repeatx2
            mov eax, [psi]
            mov [psi+8], ebx
            vmovdqa [psi+64], ymm2
         %endrep
      dec r8d
      jnz %%LL1
   %endmacro

   %define nbytes 20


%elif tcase == 27   ; integer 1 read, vector 1 read, 1 write, 256 bits

   %macro testcode 0
      mov r8d, repeatx1
      align 16
      %%LL1:
         %rep repeatx2
            mov eax, [psi]
            vmovdqa ymm1, [psi+32]
            vmovdqa [psi+64], ymm2
         %endrep
      dec r8d
      jnz %%LL1
   %endmacro

   %define nbytes 12

%elif tcase == 28   ; integer 2 read, vector 1 read, 256 bits

   %macro testcode 0
      mov r8d, repeatx1
      align 16
      %%LL1:
         %rep repeatx2
            mov eax, [psi]
            mov ebx, [psi+8]
            vmovdqa ymm1, [psi+32]
         %endrep
      dec r8d
      jnz %%LL1
   %endmacro

   %define nbytes 10


%elif tcase == 29   ; integer 2 read, vector 1 write, 256 bits

   %macro testcode 0
      mov r8d, repeatx1
      align 16
      %%LL1:
         %rep repeatx2
            mov eax, [psi]
            mov ebx, [psi+8]
            vmovdqa [psi+32], ymm1
         %endrep
      dec r8d
      jnz %%LL1
   %endmacro

   %define nbytes 10

%elif tcase == 30   ; integer 1 read, vector 2 read, 256 bits

   %macro testcode 0
      mov r8d, repeatx1
      align 16
      %%LL1:
         %rep repeatx2
            mov eax, [psi]
            vmovdqa ymm1, [psi+32]
            vmovdqa ymm2, [rdi]
         %endrep
      dec r8d
      jnz %%LL1
   %endmacro

   %define nbytes 11
   

%elif tcase == 40   ; integer 1 read, 1 write, vector 1 read, 1 write, 512 bits

   %macro testcode 0
      mov r8d, repeatx1
      align 16
      %%LL1:
         %rep repeatx2
            mov eax, [psi]
            mov [psi+8], ebx
            vmovdqa64 zmm1, [psi+64]
            vmovdqa64 [rdi], zmm2
         %endrep
      dec r8d
      jnz %%LL1
   %endmacro

   %define nbytes 18

%elif tcase == 41   ; integer 2 add, 2 move, vector 1 read, 1 write, 512 bits

   %macro testcode 0
      mov r8d, repeatx1
      align 16
      %%LL1:
         %rep repeatx2
            add eax, ebx
            add ecx, 1
            mov edx, ebx
            mov ebp, ebx
            vmovdqa64 zmm1, [psi+64]
            vmovdqa64 [rdi], zmm2
         %endrep
      dec r8d
      jnz %%LL1
   %endmacro

   %define nbytes 22

%elif tcase == 42   ; integer 1 read, 1 write, vector 1 add, 1 and, 2 move, 512 bits

   %macro testcode 0
      mov r8d, repeatx1
      align 16
      %%LL1:
         %rep repeatx2
            mov eax, [psi]
            mov [psi+8], ebx
            vpaddd zmm1, zmm2, zmm0
            vandps zmm3, zmm4, zmm5
            vmovdqa64 zmm6, zmm7
            vmovdqa64 zmm8, zmm9
         %endrep
      dec r8d
      jnz %%LL1
   %endmacro

   %define nbytes 29


%elif tcase == 60   ; 2 float add, 2 float mul, 256 bits

   %macro testcode 0
      mov r8d, repeatx1
      align 16
      %%LL1:
         %rep repeatx2
            vaddps ymm0, ymm1, ymm1
            vaddps ymm2, ymm1, ymm3
            vmulps ymm4, ymm1, ymm5
            vmulps ymm6, ymm3, ymm5
         %endrep
      dec r8d
      jnz %%LL1
   %endmacro
   
   %define nbytes 16

%elif tcase == 61   ; 2 float add, 2 fma, 256 bits

   %macro testcode 0
      mov r8d, repeatx1
      align 16
      %%LL1:
         %rep repeatx2 / 4
            vaddps ymm0, ymm1, ymm1
            vaddps ymm2, ymm1, ymm3
            VFMADD132PS ymm4, ymm1, ymm5
            VFMADD132PS ymm6, ymm3, ymm5

            vaddps ymm0, ymm1, ymm1
            vaddps ymm2, ymm1, ymm3
            VFMADD132PS ymm7, ymm1, ymm5
            VFMADD132PS ymm8, ymm3, ymm5

            vaddps ymm0, ymm1, ymm1
            vaddps ymm2, ymm1, ymm3
            VFMADD132PS ymm9, ymm1, ymm5
            VFMADD132PS ymm10, ymm3, ymm5

            vaddps ymm0, ymm1, ymm1
            vaddps ymm2, ymm1, ymm3
            VFMADD132PS ymm11, ymm1, ymm5
            VFMADD132PS ymm12, ymm3, ymm5

         %endrep
      dec r8d
      jnz %%LL1
   %endmacro

   %define nbytes 21
      
%else
   %error unknown test case tcase
%endif



;##############################################################################
;#
;#                 Extra calculations for convenience
;#
;##############################################################################

%macro extraoutput 0
Heading1        DB   "instr/clock", 0
Heading2        DB   "bytes/clock", 0
align 8

; Decide which column to base clock count and uop count on
%ifidni CPUbrand,Intel
%define ClockCol  1   ; use core clock cycles on Intel processors
%define InstCol   2   ; Instruction count
%define UopCol    4   ; 3 or 4, which has the best uop count?

%else                 ; All other CPU brands:
%define ClockCol  0   ; use RDTSC clock on all other processors
%define InstCol   1   ; Instruction count
%define UopCol    2   ; uop count
%endif

RatioOut        DD   2           ; 0: no ratio output, 1 = int, 2 = float
                DD   InstCol     ; numerator (0 = clock, 1 = first PMC, etc., -1 = none)
                DD   ClockCol    ; denominator (0 = clock, 1 = first PMC, etc., -1 = none)
                DD   1.0         ; factor, int or float according to RatioOut[0]
                
TempOut         DD   8           ; 8 = float, corrected for clock factor
                DD   0
%if modesize > 32
  RatioOutTitle DQ   Heading1      ; column heading
  TempOutTitle  DQ   Heading2      ; column heading                
%else 	
  RatioOutTitle DD   Heading1      ; column heading
  TempOutTitle  DD   Heading2      ; column heading                
%endif
                
%endmacro       


%macro testafter3 0
; calculate bytes per clock
%if modesize > 32

    xor     r14d, r14d
TESTAFTERLOOP:

    mov      eax, repeatx1 * repeatx2 * nbytes + 7                       ; calculate number of bytes
    cvtsi2ss xmm0, eax
    mov      ebx, [r13 + r14*4 + (ClockCol-1)*4*REPETITIONS0+(PMCResults-ThreadData)] ; recall clock count
    cvtsi2ss xmm1, ebx
    divss    xmm0, xmm1                                                              ; divide
    movss    [r13 + r14*4 + (CountTemp-ThreadData)], xmm0                            ; store in CountTemp

    inc     r14d
    cmp     r14d, REPETITIONS0
    jb      TESTAFTERLOOP

%endif
%endmacro 
