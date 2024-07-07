;----------------------------------------------------------------------------
;                       decode_prefix.inc                2018-08-08 Agner Fog
;
;          PMC Test program for decoding instructions that generate multiple uops
;                           NASM syntax
;
;
; The following macros can be defined on the command line or in include files:
; 
; tcase:    Test case:
;           1. length changing prefix 66
;           2. length changing prefix REX.W
;           3. length changing prefix 67
;           4. lzcnt 66 F3 0F
;           5. lzcnt F3 48 0F
;           6. 66 0F long nop
;           7. 66 0F 38 movapd
;           8. 66 0F 38 pabsw
;           9. segment + REX mov rax,rbx
;          10. segment + VEX2 vmovaps
;          11. segment + VEX3 vmovaps 
;          12. segment + EVEX4 vmovaps xmm1 {k1}, xmm2
;
; num_pref = number of extra prefixes, must be at least 1
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

%ifndef num_pref
   %define num_pref 1
%endif

%ifndef CPUbrand
   %define CPUbrand Intel
%endif

%ifndef ifamily
   %define ifamily 6
%endif

%ifndef ifamily
   %define imodel 0
%endif

%define familymodel (ifamily * 0x100 + imodel)


%macro segprefixes 1  ; pad instruction with dummy segment prefixes
   times %1 db 3EH
%endmacro

%macro testinit2 0
   mov ecx, 1
   xor edi, edi
%endmacro

%macro testinit3 0   ; needs to reset rsi if lodsb instruction used
   %ifidni CPUbrand,AMD
      %if tcase == 9 || tcase == 10
         mov reg5, reg4
      %endif
   %endif
%endmacro

; define long nops
%ifndef noptype
   %define noptype 2
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
; calculate factor RatioOut[3] for clocks per 16 bytes
%endmacro       

%macro testafter3 0
; calculate bytes per clock

%if modesize > 32
    xor     r14d, r14d
TESTAFTERLOOP:

    mov      eax, repeat1 * (repeat2 * ilen + 8)                                     ; calculate number of bytes
    cvtsi2ss xmm0, eax
    mov      ebx, [r13 + r14*4 + (ClockCol-1)*4*REPETITIONS0+(PMCResults-ThreadData)] ; recall clock count
    cvtsi2ss xmm1, ebx
    divss    xmm0, xmm1                                                              ; divide
    movss    [r13 + r14*4 + (CountTemp-ThreadData)], xmm0                            ; store in CountTemp

    inc     r14d
    cmp     r14d, REPETITIONS0
    jb      TESTAFTERLOOP

%else ; 32 bit mode

    xor     edi, edi
    mov     ebx, [esp+4]
TESTAFTERLOOP:

    mov      eax, repeat1 * (repeat2 * ilen + 8)                                     ; calculate number of bytes
    cvtsi2ss xmm0, eax
    mov      ecx, [ebx + edi*4 + (ClockCol-1)*4*REPETITIONS0+(PMCResults-ThreadData)] ; recall clock count
    cvtsi2ss xmm1, ecx
    divss    xmm0, xmm1                                                              ; divide
    movss    [ebx + edi*4 + (CountTemp-ThreadData)], xmm0                            ; store in CountTemp
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

%if tcase == 1     ; length changing prefix 66

  %macro testcode 0
  %rep    num_pref-1
  db 66H
  %endrep  
  mov ax,1000h
  %rep    num_pref-1
  db 66H
  %endrep  
  mov bx,2000h
  %endmacro

  %assign ilen  8+2*num_pref
    
%elif tcase == 2   ; length changing prefix REX.W

  %macro testcode 0
  %rep    num_pref-1
  db 3EH
  %endrep  
  mov rax,1000000000000000h
  %endmacro

  %assign ilen  9+num_pref
    
%elif tcase == 3      ; length changing prefix 67, 32 bit mode
  
  %macro testcode 0
  %rep    num_pref-1
  db 67H
  %endrep  
  lea eax,[bx+1000h]
  ;lea cx,[bx+1000h]
  %endmacro

  %assign ilen  4+num_pref
    
%elif tcase == 4      ; lzcnt 66 F3 0F
  
  %macro testcode 0
  %rep    num_pref-1
  db 66H
  %endrep  
  lzcnt   ax, bx
  %rep    num_pref-1
  db 66H
  %endrep  
  lzcnt   cx, dx
  %endmacro

  %assign ilen  8+2*num_pref

%elif tcase == 5      ; lzcnt F3 REX 0F
  
  %macro testcode 0
  %rep    num_pref-1
  db 0F3H
  %endrep  
  lzcnt   rax, rbx
  %endmacro

  %assign ilen  4+num_pref
    
%elif tcase == 6      ; 66 0F long nop

  %macro testcode 0
  %rep    num_pref
  db 66H
  %endrep  
  db 0FH, 1FH, 0C0H
  %endmacro

  %assign ilen  3 + num_pref

%elif tcase == 7      ; 66 0F movapd
  
  %macro testcode 0
  %rep    num_pref
  db 66H
  %endrep  
  movaps  xmm1, xmm2  ; becomes movapd xmm1,xmm2
  %endmacro

  %assign ilen  3 + num_pref
    
%elif tcase == 8      ; 66 0F 38 pabsw

  %macro testcode 0
  %rep    num_pref
  db 66H
  %endrep  
  DB 0FH, 38H, 1DH, 0CAH  ; pabsw  xmm1, xmm2 (not allowed without 66 prefix)
  %endmacro

  %assign ilen  4 + num_pref

%elif tcase == 9      ; segment + REX
  
  %macro testcode 0
  %rep    num_pref
  db 3EH    ; DS segment prefix
  %endrep  
  mov rax,rbx
  %endmacro

  %assign ilen  3 + num_pref
  
%elif tcase == 10     ; segment + VEX2
  
  %macro testcode 0
  %rep    num_pref
  db 3EH    ; DS segment prefix
  %endrep  
  vmovaps xmm1, xmm2
  %endmacro

  %assign ilen  4 + num_pref
  
%elif tcase == 11     ; segment + VEX3

  %macro testcode 0
  %rep    num_pref
  db 3EH    ; DS segment prefix
  %endrep  
  vmovaps xmm8, xmm9
  %endmacro

  %assign ilen  5 + num_pref

%elif tcase == 12     ; segment + EVEX4
  
  %macro testcode 0
  %rep    num_pref
  db 3EH    ; DS segment prefix
  %endrep  
  vmovaps xmm1 {k1}, xmm2
  %endmacro

  %assign ilen  6 + num_pref


%else
  
  %error unknown test case tcase
    
%endif
