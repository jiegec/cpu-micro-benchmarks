;----------------------------------------------------------------------------
;                       decode_prefix.inc                2018-08-08 Agner Fog
;
;          PMC Test program testing various instructions to see how many 
;              can be contained in uop cache
;
;
; The following macros can be defined on the command line or in include files:
; 
; tcase:    Test case:
;           1. nop8
;           2. nop12
;           3. 2*add r,[r+o] / nop12
;           4. 2* cmp r,[r+r*4] / nop12
;           5. cmp r,[r+r*4+o] / nop13
;           6. 2*xchg r,r / nop12
;           7. vmovaps y,y / nop8
;           8. xchg r,r / vmovaps y,y
;
; CPUbrand:   AMD, Intel, VIA
; ifamily:    CPU family number
; imodel:     CPU model number
; 
;
; (c) Copyright 2018 by Agner Fog. GNU General Public License www.gnu.org/licenses
;-----------------------------------------------------------------------------
; Define any undefined macros

%ifndef tcase
   %define tcase 1
%endif

%ifndef CPUbrand
   %define CPUbrand Intel
%endif

%ifndef ifamily
   %define ifamily 6
%endif

%ifndef imodel
   %define imodel 0
%endif

%define familymodel (ifamily * 0x100 + imodel)


; define long nops
%ifndef noptype
   %define noptype 2
%endif

%include "nops.inc"


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
                
TempOut         DD   6           ; 6 = float
                DD   0
%if modesize > 32
RatioOutTitle   DQ   Heading1      ; column heading
TempOutTitle    DQ   Heading2      ; column heading                
%else
RatioOutTitle   DD   Heading1      ; column heading
TempOutTitle    DD   Heading2      ; column heading                
%endif
                
%endmacro       

%macro testinit1 0
    lea rdi,[rsi+16]
    xor ebp,ebp
%endmacro       

%macro testafter3 0
; calculate bytes per clock

%if modesize > 32
    xor     r14d, r14d
TESTAFTERLOOP:

    mov      eax, repeat1 * (repeat2 * ilen + 8)                                     ; calculate number of bytes
    cvtsi2ss xmm0, eax
    mov      edx, [r13 + r14*4 + (ClockCol-1)*4*REPETITIONS0+(PMCResults-ThreadData)] ; recall clock count
    cvtsi2ss xmm1, edx
    divss    xmm0, xmm1                                                              ; divide
    movss    [r13 + r14*4 + (CountTemp-ThreadData)], xmm0                            ; store in CountTemp

    inc     r14d
    cmp     r14d, REPETITIONS0
    jb      TESTAFTERLOOP

%else ; 32 bit mode

    xor     edi, edi
    mov     edx, [esp+4]
TESTAFTERLOOP:

    mov      eax, repeat1 * (repeat2 * ilen + 8)                                     ; calculate number of bytes
    cvtsi2ss xmm0, eax
    mov      ecx, [edx + edi*4 + (ClockCol-1)*4*REPETITIONS0+(PMCResults-ThreadData)] ; recall clock count
    cvtsi2ss xmm1, ecx
    divss    xmm0, xmm1                                                              ; divide
    movss    [edx + edi*4 + (CountTemp-ThreadData)], xmm0                            ; store in CountTemp
    inc     edi
    cmp     edi, REPETITIONS0
    jb      TESTAFTERLOOP
%endif
%endmacro 


;##############################################################################
;#
;#                 Test code macro
;#
;##############################################################################

; main testcode macro

%if tcase == 1     ; nop8

  %macro testcode 0
  nop8
  nop8
  %endmacro

  %assign ilen  16
    
%elif tcase == 2   ; nop12

  %macro testcode 0
  nop12
  nop12
  %endmacro

  %assign ilen  24
    
%elif tcase == 3      ; add r,[r+o]
  
  %macro testcode 0
  add eax,[rsi+200h]
  add ebx,[rdi+200h]
  nop12
  %endmacro

  %assign ilen  24
    
%elif tcase == 4      ; 2* cmp r,[r+r*4] / nop12
  
  %macro testcode 0
  cmp eax,[rsi+rbp*4] ; 3
  cmp ecx,[rdi+rbp*4] ; 3
  nop12
  %endmacro

  %assign ilen  18

%elif tcase == 5      ; cmp r,[r+r*4+o] / nop13
  
  %macro testcode 0
  cmp eax,[rsi+rbp*4+200h]
  nop13  
  %endmacro

  %assign ilen  20
    
%elif tcase == 6      ; 2*xchg r,r / nop12

  %macro testcode 0
  xchg rbx,rcx
  xchg rdx,rdi
  nop12  
  %endmacro

  %assign ilen  18

%elif tcase == 7      ; vmovaps y,y / nop8
  
  %macro testcode 0
  vmovaps ymm1,ymm2 ; 4
  vmovapd ymm3,ymm4
  nop8
  %endmacro

  %assign ilen  16
    
%elif tcase == 8      ; xchg r,r / vmovaps y,y

  %macro testcode 0
  xchg ebx,ecx      ; 2
  vmovaps ymm1,ymm2 ; 4
  nop10
  %endmacro

  %assign ilen  16



%else
  
  %error unknown test case tcase
    
%endif
