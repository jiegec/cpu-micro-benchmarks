;----------------------------------------------------------------------------
;               decoder_throughput.inc                2021-03-16 Agner Fog
;
; PMC Test program for testing any loop buffer or microop cache
; Uses different instructions to see if they fit in the microop cache

; Parameters:
;
; tcase:     Test case:
;            1: short NOPs
;            2: long NOPs, 12 bytes
;            3: long NOPs, 15 bytes
;            4: mixed integer and vector instructions, 6 instructions
;            5: 3 short nops and one alu/branch pair
;            6: 4 short nops and one alu/branch pair
;            7: 2 short nops and two alu/branch pairs
;            8: 3 short nops and two alu/branch pairs
;            9: 4 short nops and two alu/branch pairs
;           10: 3 short nops and three alu/branch pairs
;           11: two alu/branch pairs
;
; numinstr:  Total number of instructions in loop
; 
; nopsize:   Size of NOP instructions
; 
; noptype:   Type of NOPs 
;
; (c) Copyright 2021 by Agner Fog. 
; GNU General Public License www.gnu.org/licenses


;-----------------------------------------------------------------------------

%ifndef tcase
   %define tcase   2
%endif

%ifndef numinstr
   %define numinstr   10000
%endif

%ifndef noptype
   %define noptype  2
%endif

%ifndef nopsize
   %define nopsize  1
%endif

%include "nops.inc"

; number of instructions in macro
%if tcase < 4
   %assign ninstruct  1
%elif tcase == 4
   %assign ninstruct  6
%elif tcase == 5
   %assign ninstruct  5
%elif tcase == 6
   %assign ninstruct  6
%elif tcase == 7
   %assign ninstruct  6
%elif tcase == 8
   %assign ninstruct  7
%elif tcase == 9
   %assign ninstruct  8
%elif tcase == 10
   %assign ninstruct  9
%elif tcase == 11
   %assign ninstruct  4
%endif

; test loop counts
%assign repeat1 100
%assign repeat2 (numinstr/ninstruct)



;##############################################################################
;#
;#                 Test code macro
;#
;##############################################################################



%macro testinit1 0
   xor  ebp, ebp
   mov  edi, 1
%endmacro

%macro testinit3 0
   xor  eax, eax
   %if tcase == 99  ; Boundary crossing. not used
      nop6
      nop6
   %endif   
%endmacro

; main testcode macro
%macro testcode 0


%if tcase == 1

    nop

%elif case == 2

    nop12

%elif case == 3

    nop15

%elif case == 4 ; mixed integer and vector instructions, 6 instructions

    mov eax, 1
    mov ebx, 2
    cmp ecx, 3
    add edx, 4
    vpaddd xmm1,xmm2,xmm3
    vorps  xmm4,xmm5,xmm6

%elif case == 5 ; 3  nops and one alu/branch pair

    nop
    nop
    cmp eax,eax
    jne $+3
    nop

%elif case == 6 ; 4  nops and one alu/branch pair

    nop
    nop
    nop
    cmp eax,eax
    jne $+3
    nop

%elif case == 7 ; 2  nops and two alu/branch pairs

    cmp eax,eax
    jne $+3
    nop
    cmp ebx,ebx
    jne $+3
    nop

%elif case == 8 ; 3  nops and two alu/branch pairs

    nop
    cmp eax,eax
    jne $+3
    nop
    cmp ebx,ebx
    jne $+3
    nop

%elif case == 9 ; 4  nops and two alu/branch pairs

    nop
    nop
    cmp eax,eax
    jne $+3
    nop
    cmp ebx,ebx
    jne $+3
    nop

%elif case == 10 ; 3  nops and three alu/branch pairs

    cmp eax,eax
    jne $+3
    nop
    cmp ebx,ebx
    jne $+3
    nop
    cmp ecx,ecx
    jne $+3
    nop
   
%elif case == 11 ; two alu/branch pairs

    cmp eax,eax
    jne $+4
    cmp ebx,ebx
    jne $+4
   
%else
   %error unknown case
%endif


%endmacro ; testcode


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
                
TempOut         DD   0           ; 8 = float, corrected for clock factor
                DD   0
%if modesize > 32
  RatioOutTitle DQ   Heading1      ; column heading
  TempOutTitle  DQ   Heading2      ; column heading                
%else 	
  RatioOutTitle DD   Heading1      ; column heading
  TempOutTitle  DD   Heading2      ; column heading                
%endif
                
%endmacro       

