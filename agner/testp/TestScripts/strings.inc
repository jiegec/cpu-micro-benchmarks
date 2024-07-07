;----------------------------------------------------------------------------
;                       strings.inc                    2012-01-30 Agner Fog
;
; Include file for testing string instructions 
;
; The following macros can be defined on the command line or in include files:
; 
; instruct:     The name of a single instruction to test
; 
; repp:         Repeat prefix: rep, repe, or undefined
;
; count:        Repeat count (only if repeat prefix)
;
; misalign:     Alignment offset
;
; (c) Copyright 2012 by Agner Fog. GNU General Public License www.gnu.org/licenses
;-----------------------------------------------------------------------------
; Define any undefined macros

%ifndef instruct
   %define instruct  lodsb  ; default instruction 
%endif

%ifndef count
   %define count  100
%endif

%ifndef misalign
   %define misalign  0      ; default misalignment
%endif

%ifndef regsize              ; default: define registers as 256 bit
   %define regsize   32
%endif

%define repeat2  1

%define repeatinstr 100

;-----------------------------------------------------------------------------
;                 User data
;-----------------------------------------------------------------------------

%macro testdata 0
   times 100000  DD 0 
%endmacro


;##############################################################################
;#
;#                 Test code macro
;#
;##############################################################################

        
%macro testcode 0
   %ifdef  repp   ; repeat prefix used        
      mov ebx, count
      lea r8, [UserData+misalign]
      lea r9, [UserData+misalign+240h]
      xor eax, eax
      %rep repeatinstr
         mov rsi, r8
         mov rdi, r9
         mov ecx, ebx
         repp instruct
      %endrep 

   %else   ; no repeat prefix
      lea rsi, [UserData+misalign]
      lea rdi, [UserData+misalign+240h]
      %rep repeatinstr
         instruct
      %endrep 

   %endif
%endmacro


;##############################################################################
;#
;#                 Extra calculations for convenience
;#
;##############################################################################

%ifdef  repp   ; repeat prefix used

%macro extraoutput 0
Heading1        DB   "clock/16B", 0
Heading2        DB   "uops/16B", 0
align 8

; Decide which column to base clock count and uop count on
%ifidni CPUbrand,Intel
%define ClockCol  1   ; use core clock cycles on Intel processors
%define UopCol    4   ; 3 or 4, which has the best uop count?

%else                 ; All other CPU brands:
%define ClockCol  0   ; use RDTSC clock on all other processors
%define UopCol    2   ; which column has uop count
%endif

RatioOut        DD   2           ; 0: no ratio output, 1 = int, 2 = float
                DD   ClockCol    ; numerator (0 = clock, 1 = first PMC, etc., -1 = none)
                DD  -1           ; denominator (0 = clock, 1 = first PMC, etc., -1 = none)
                DD   0           ; factor calculated later, int or float according to RatioOut[0]
                
TempOut         DD   6           ; 6 = float
                DD   0
RatioOutTitle DQ   Heading1      ; column heading
TempOutTitle  DQ   Heading2      ; column heading                
                
%endmacro       

%macro testinit1 0
; calculate factor RatioOut[3] for clocks per 16 bytes
    mov eax, 16
    mov ebx, count
    mov ecx, repeatinstr * (regsize/8)
    cvtsi2ss xmm0, eax
    cvtsi2ss xmm1, ebx
    cvtsi2ss xmm2, ecx
    mulss    xmm1, xmm2
    divss    xmm0, xmm1
    movss    [RatioOut+12], xmm0
%endmacro       

%macro testafter3 0
; calculate microops per 16 bytes

    xor     r14d, r14d
TESTAFTERLOOP:

    mov      eax, [r13 + r14*4 + (UopCol-1)*4*REPETITIONS0+(PMCResults-ThreadData)] ; recall uop count
    cvtsi2ss xmm0, eax
    mulss    xmm0, [RatioOut+12]                                                    ; multiply by factor
    
    movss    [r13 + r14*4 + (CountTemp-ThreadData)], xmm0                           ; store in CountTemp

    inc     r14d
    cmp     r14d, REPETITIONS0
    jb      TESTAFTERLOOP

%endmacro 

%endif      
