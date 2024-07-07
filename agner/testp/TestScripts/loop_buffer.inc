;----------------------------------------------------------------------------
;        loop_buffer.inc                          2020-08-26 Agner Fog
;
; PMC Test program for testing any loop buffer or microop cache
;
; Constants to be defined:
; 
; nopsize:   Size of NOP instructions. (1 - 15)
; 
; noptype:   2: long NOPs (0F 1F ...)
;            3: 66 NOPs (simple NOP with up to 14 operand size prefixes)
; 
; repeat1:   Number of loop repetitions
;
; repeat2:   Number of NOPs in loop
;
; (c) Copyright 2013-2020 by Agner Fog. GNU General Public License www.gnu.org/licenses
;-----------------------------------------------------------------------------
; Define any undefined macros

; define long nops
%ifndef noptype
   %define noptype 2
%endif

%include "nops.inc"

%if nopsize == 1
   %macro testcode 0
      nop1
   %endmacro   
%elif nopsize == 2
   %macro testcode 0
      nop2
   %endmacro
%elif nopsize == 3
   %macro testcode 0
      nop3
   %endmacro
%elif nopsize == 4
   %macro testcode 0
      nop4
   %endmacro
%elif nopsize == 5
   %macro testcode 0
      nop5
   %endmacro
%elif nopsize == 6
   %macro testcode 0
      nop6
   %endmacro
%elif nopsize == 7
   %macro testcode 0
      nop7
   %endmacro
%elif nopsize == 8
   %macro testcode 0
      nop8
   %endmacro
%elif nopsize == 9
   %macro testcode 0
      nop9
   %endmacro
%elif nopsize == 10
   %macro testcode 0
      nop10
   %endmacro
%elif nopsize == 11
   %macro testcode 0
      nop11
   %endmacro
%elif nopsize == 12
   %macro testcode 0
      nop12
   %endmacro
%elif nopsize == 13
   %macro testcode 0
      nop13
   %endmacro
%elif nopsize == 14
   %macro testcode 0
      nop14
   %endmacro
%elif nopsize == 15
   %macro testcode 0
      nop15
   %endmacro
%else
   %error wrong nop size nopsize
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
RatioOutTitle DQ   Heading1      ; column heading
TempOutTitle  DQ   Heading2      ; column heading                
                
%endmacro       

%macro testinit1 0
; calculate factor RatioOut[3] for clocks per 16 bytes
%endmacro       

%macro testafter3 0
; calculate bytes per clock

    xor     r14d, r14d
TESTAFTERLOOP:

    mov      eax, repeat1 * (repeat2 * nopsize + 8)                                            ; calculate number of bytes
    cvtsi2ss xmm0, eax
    mov      ebx, [r13 + r14*4 + (ClockCol-1)*4*REPETITIONS0+(PMCResults-ThreadData)] ; recall clock count
    cvtsi2ss xmm1, ebx
    divss    xmm0, xmm1                                                              ; divide
    movss    [r13 + r14*4 + (CountTemp-ThreadData)], xmm0                            ; store in CountTemp

    inc     r14d
    cmp     r14d, REPETITIONS0
    jb      TESTAFTERLOOP

%endmacro 
