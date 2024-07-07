;----------------------------------------------------------------------------
;                       decode_double.inc 
;
;          PMC Test program for decoding instructions that generate multiple uops
;                           NASM syntax
;
; Last modified: 2020-08-16
;
; Note: This script may need individual adjustment for each processor type!
;
; The following macros can be defined on the command line or in include files:
; 
; tcase:    Test case:
;           1. 1-1-1-1 single instructions
;           2. 2-2     double instructions, integer
;           3. 2-2     double instructions, vector
;           4. 2-2     double instructions, mixed type
;           5. 2-1-2-1
;           6. 2-1-1
;           7. 2-1-1-1
;           8. 2-2-1
;           9. 3-1
;          10. 3-1-1
;          11. 4-1
;          12. 3-2-1-2
;          13. 3-1-2-1
;          14. 3-1-1-1-1 last instruction OR, not fuseable 
;          15. 3-1-1-1-1 last instruction AND, fuseable

;
; CPUbrand:   AMD, Intel, VIA
; ifamily:    CPU family number
; imodel:     CPU model number
; 
;
; (c) Copyright 2013-2018 by Agner Fog. GNU General Public License www.gnu.org/licenses
;-----------------------------------------------------------------------------
; Define any undefined macros

%ifndef tcase
   %define tcase 2
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
   ;   %if tcase == 9 || tcase == 10
   ;      mov reg5, reg4
   ;   %endif
   lea psi, [UserData]
   %endif
%endmacro

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
Heading2        DB   "uops/clock", 0
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
; calculate microops per clock
%if modesize > 32  ; to do: make 32-bit version
    xor     r14d, r14d
TESTAFTERLOOP:

    mov      eax, [r13 + r14*4 + (UopCol-1)*4*REPETITIONS0+(PMCResults-ThreadData)]  ; recall uop count
    cvtsi2ss xmm0, eax
    mov      ebx, [r13 + r14*4 + (ClockCol-1)*4*REPETITIONS0+(PMCResults-ThreadData)]  ; recall clock count
    cvtsi2ss xmm1, ebx
    divss    xmm0, xmm1
    movss    [r13 + r14*4 + (CountTemp-ThreadData)], xmm0                           ; store in CountTemp

    inc     r14d
    cmp     r14d, REPETITIONS0
    jb      TESTAFTERLOOP
%endif
%endmacro 

%macro testdata 0
        times 100000H  DB 0
%endmacro


;##############################################################################
;#
;#                 Test code macro
;#
;##############################################################################

; main testcode macro
%macro testcode 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;      
;                            Intel
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;      

%ifidni CPUbrand,Intel

  %if familymodel >= 0x063C   ; Intel Haswell and later

  %if tcase == 1      ; 1-1-1-1 single instructions
    inc eax
    mov ebx,ecx
    movdqa xmm0,xmm1
    vaddps xmm2,xmm3,xmm3
    
  %elif tcase == 2      ; 2-2     double instructions, integer
  
    segprefixes 1
    cmove ebx, eax
    segprefixes 1
    cmovne ecx, eax
    segprefixes 1
    cmove edx, eax
    segprefixes 1
    cmovne edi, eax
    
  %elif tcase == 3      ; 2-2     double instructions, xmm
  
    ;cvtss2sd xmm0, xmm1
    ;cvtss2sd xmm2, xmm3
    segprefixes 1
    ptest xmm0,xmm0
    segprefixes 1
    ptest xmm1,xmm1
    
  %elif tcase == 4      ; 2-2     double instructions, mixed type
  
    segprefixes 1
    cmove eax, ebx
    ptest xmm0, xmm1
    segprefixes 1
    cmove ecx, edx
    ptest xmm0, xmm1

  %elif tcase == 5      ; 2-1-2-1
  
    segprefixes 1
    cmove eax, ebx
    nop
    ptest xmm0, xmm1
    mov edx, esi
    segprefixes 1
    cmove ecx, edx
    nop
    ptest xmm2, xmm3
    mov edx, esi
    
  %elif tcase == 6      ; 2-1-1
  
    ptest xmm0, xmm1
    inc ecx
    inc edx
  
  %elif tcase == 7      ; 2-1-1-1
  
   ; segprefixes 1
    ptest xmm0, xmm1
    inc eax
    inc ebx
    mov ecx, edx
    
  %elif tcase == 8      ; 2-2-1

    segprefixes 1
    cmove eax, ebx
    ptest xmm0, xmm1
    mov edx,[rsi]
  
  %elif tcase == 9      ; 3-1
  
    segprefixes 1
    movbe ax,[rsi]
    mov ax,[rsi]
    segprefixes 1
    mov ebx,ecx
  
  %elif tcase == 10     ; 3-1-1
  
    segprefixes 1
    movbe ax,[rsi]
    mov ebx,ecx
    segprefixes 1
    nop
  
  %elif tcase == 11     ; 4-1

    ;dppd xmm0,[rsi],0
    segprefixes 1
    vzeroupper
    nop

  %elif tcase == 12     ; 3-2-1-2
  
    movbe ax,[rsi]
    segprefixes 1
    ptest xmm1, xmm2
    segprefixes 1
    nop
    segprefixes 1
    cmove ecx, edx
  
  %elif tcase == 13     ; 3-1-2-1

;    segprefixes 1
;    vzeroupper
    movbe ax,[rsi]
    segprefixes 1
    nop
;    ptest xmm0, xmm1
    segprefixes 1
    cmove ecx, edx
    segprefixes 1
    nop
    
  %elif tcase == 14    ; 3-1-1-1-1 last instruction OR, not fuseable 

    movbe ax,[rsi]
    or ebx,ebx
    or ecx,ecx
    or edx,edx
    or edi,edi
;	jnz $-2

  %elif tcase == 15    ; 3-1-1-1-(1+1) last instruction AND+JNZ fused

    movbe ax,[rsi]
    or ebx,ebx
    or ecx,ecx
    or edx,edx
    and edi,edi
	jnz $-2

  %elif tcase == 16    ; 3-1-1-(1+1)-1 3rd instruction AND+JNZ fused

    movbe ax,[rsi]
    or ebx,ebx
    or ecx,ecx
    and edi,edi
	jnz $-2
    or edx,edx

  %else
  
    %error unknown test case tcase
    
  %endif
    
  %else
  ; Intel Ivy Bridge or earlier
  
  %if tcase == 1      ; 1-1-1-1 single instructions
    inc eax
    mov ebx,ecx
    ;movdqa xmm0,xmm1
    nop
    mov edx,[rsi]
    
  %elif tcase == 2      ; 2-2     double instructions, integer
  
    segprefixes 1
    cmove ebx, eax
    segprefixes 1
    cmovne ecx, eax
    
  %elif tcase == 3      ; 2-2     double instructions, xmm
  
    blendps xmm0,[rsi],1
    shufps xmm1,[rsi+10h],1    
    
  %elif tcase == 4      ; 2-2     double instructions, mixed type
  
    cmove rax, rbx
    blendps xmm0,[rsi],1

  %elif tcase == 5      ; 2-1-2-1
  
    cmove rax, rbx
    inc ecx
    blendps xmm0,[rsi],1
    inc edx
    
  %elif tcase == 6      ; 2-1-1
  
    blendps xmm0,[rsi],1
    inc ecx
    nop
  
  %elif tcase == 7      ; 2-1-1-1
  
    blendps xmm0,[rsi],1
    inc eax
    nop
    mov ecx, edx
    
  %elif tcase == 8      ; 2-2-1

    cmove rax, rbx
    blendps xmm0,[rsi],1
    mov edx,[rsi]
  
  %elif tcase == 9      ; 3-1
  
    xchg rax,rbx
    mov ecx,[rsi]
  
  %elif tcase == 10     ; 3-1-1
  
    xchg rax,rbx
    mov ecx,[rsi]
    nop
  
  %elif tcase == 11     ; 4-1
  
    shrd eax,ebx,cl
    nop

  %elif tcase == 12     ; 3-2-1-2
  
    xchg rax,rbx
    blendps xmm0,[rsi],1
    nop
    cmove r8, r9
  
  %elif tcase == 13     ; 3-1-2-1

    xchg rax,rbx
    nop
    blendps xmm0,[rsi],1
    mov ecx,[rsi]
    
  %elif tcase == 14    ; 3-1-1-1-(1-1) last instruction OR, not fuseable 

    xchg r8,r9
    nop5
    nop5
    or ecx,ecx
    or edi,edi
;	jnz $-2


  %elif tcase == 15    ; 3-1-1-1-(1+1) last instruction AND+JNZ

    xchg r8,r9
    nop4
    nop4
    or ecx,ecx
    and edi,edi
	jnz $-2

  %elif tcase == 16    ; 3-1-1-(1+1)-1 3rd instruction AND+JNZ

    xchg r8,r9
    nop4
    or ecx,ecx
    and edi,edi
	jnz $-2
    nop4

  %else
  
    %error unknown test case tcase
    
  %endif  ; tcase
  

  %endif  ; familymodel, Intel

  
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;      
;                            AMD
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;      
  
%elifidni CPUbrand,AMD

  %if familymodel >= 0x1502   ; AMD Piledriver

  %if tcase == 1      ; 1-1-1-1 single instructions
    inc eax
    mov ebx,ecx
    movdqa xmm0,xmm1
    por xmm2,xmm3 
    
  %elif tcase == 2      ; 2-2     double instructions, integer
  
    blsmsk  eax, ebx
    blsmsk  ecx, edx
    
  %elif tcase == 3      ; 2-2     double instructions, ymm
  
    vmovaps ymm0, ymm1
    vaddps  ymm2, ymm3, ymm3
    
  %elif tcase == 4      ; 2-2     double instructions, mixed type
  
    blsmsk  eax, ebx
    vmovaps ymm0, ymm1

  %elif tcase == 5      ; 2-1-2-1
  
    blsmsk  eax, ebx
    nop
    blsmsk  ecx, ebx
    mov edx, esi
    
  %elif tcase == 6      ; 2-1-1
  
    blsmsk  eax, ebx
    inc ecx
    nop
  
  %elif tcase == 7      ; 2-1-1-1
  
    blsmsk  eax, ebx
    mov ecx, edx
    nop
    nop

  %elif tcase == 8      ; 2-2-1
  
    blsmsk  eax, ebx
    blsmsk  ecx, ebx
    nop
  
  %elif tcase == 9      ; 3-1
  
    lodsb
    ;imul cx
    nop    
  
  %elif tcase == 10     ; 3-1-1
  
    lodsb
    ;imul cx
    nop    
    nop    
  
  %elif tcase == 11     ; 4-1
  
    ;vcvtpd2ps xmm1,ymm2    
    ;vcvtdq2pd ymm1, xmm2
    ;nop
    into
    nop

  %else

    ; test cases not defined
    %define repeat0 1
    
  %endif

  %else   ; AMD Bulldozer or earlier

  %if tcase == 1      ; 1-1-1-1 single instructions
    inc eax
    mov ebx,ecx
    movdqa xmm0,xmm1
    nop
    
  %elif tcase == 2      ; 2-2     double instructions, integer
  
    xchg  ebx, ecx
    sahf
    
  %elif tcase == 3      ; 2-2     double instructions, ymm
  
    vmovaps ymm0, ymm1
    vmovaps ymm2, ymm3
    
  %elif tcase == 4      ; 2-2     double instructions, mixed type
  
    cwd
    vmovaps ymm0, ymm1

  %elif tcase == 5      ; 2-1-2-1
  
    cwd
    nop
    vmovaps ymm0, ymm1
    nop
    
  %elif tcase == 6      ; 2-1-1
  
    vmovaps ymm0, ymm1
    inc ecx
    nop
  
  %elif tcase == 7      ; 2-1-1-1
  
    vmovaps ymm0, ymm1
    mov ecx, edx
    nop
    nop

  %elif tcase == 8      ; 2-2-1
  
    cwd
    vmovaps ymm0, ymm1
    nop
  
  %elif tcase == 9      ; 3-1
  
    vhaddps xmm1,xmm2,xmm3
    nop    
  
  %elif tcase == 10     ; 3-1-1
  
    vhaddps xmm1,xmm2,xmm3
    nop    
    nop    
  
  %elif tcase == 11     ; 4-1
  
    vhaddps xmm1,xmm2,[psi]
    nop

  %else

    ; test cases not defined
    %define repeat0 1
    
  %endif  ; tcase

  %endif  ; familymodel

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;      
;                            VIA
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;      

%elifidni CPUbrand,VIA

  %error not implemented yet

%else

  %error unknown CPU brand

%endif
   

%endmacro    ; testcode

;  default test loops
;%define repeat1 1
;%define repeat2 1

