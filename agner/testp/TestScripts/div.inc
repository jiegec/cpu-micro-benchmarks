;----------------------------------------------------------------------------
;                       div.inc                       2012-01-26 Agner Fog
;
;           PMC Test program for integer division instructions
;                           YASM syntax
;
; The following macros can be defined on the command line or in include files:
; 
; instruct:     The name of a single instruction to test
; 
; regsize:      Register size: 8, 16, 32, 64
;
; divhi:        Dividend, high word
; 
; divlo:        Dividend, low word
;
; divisor:      Divisor, must be > divhi
;
; tmode:        L:  Latency
;               T:  Throughput
;               M:  Throughput with memory operand
;
; 
; (c) Copyright 2012 by Agner Fog. GNU General Public License www.gnu.org/licenses
;-----------------------------------------------------------------------------
; Define any undefined macros

%ifndef instruct
   %define instruct  nop  ; default instruction is NOP
%endif

%ifndef tmode
   %define tmode  L          ; default: measure latency
%endif

%ifndef regsize              ; default: define registers as 32 bit
   %define regsize   32
%endif

%ifndef divhi   ; define value of dividend
   %define divhi 0
%endif

%ifndef divlo   ; define value of dividend
   %define divlo 1
%endif

%ifndef divisor   ; define value of divisor
   %define divisor 1
%endif

; define registers

%if regsize == 8
   %define rglo  al
   %define rghi  ah
   %define rg1   bl
   %define rg2   bh
   %define rg3   cl
   %define sizeptr byte
%elif regsize == 16
   %define rglo  ax
   %define rghi  dx
   %define rg1   bx
   %define rg2   cx
   %define rg3   di
   %define sizeptr word
%elif regsize == 32
   %define rglo  eax
   %define rghi  edx
   %define rg1   ebx
   %define rg2   ecx
   %define rg3   edi
   %define sizeptr dword
%elif regsize == 64
   %define rglo  rax
   %define rghi  rdx
   %define rg1   rbx
   %define rg2   rcx
   %define rg3   rdi
   %define sizeptr qword
%else
   %error unknown register size
%endif


;##############################################################################
;#
;#                 Test code macro
;#
;##############################################################################

%macro testcode 0

        mov rglo, 0
        mov rg1, divlo
        mov rg2, divhi
        mov rg3, divisor
        mov sizeptr [rsi], rg3

        mov ebp,100
        align 32
Testloop1:

%IFIDNI tmode, L         ; measure latency

   %rep 100        
        or rglo, rg1
        mov rghi, rg2         
        instruct rg3
   %endrep

%ELIFIDNI tmode, T         ; measure throughput

   %rep 100
      %if regsize == 8
        mov eax, ebx        ; avoid partial register dependence
        nop
      %elif regsize == 16
        mov eax, ebx
        mov edx, ecx
      %else
        mov rglo, rg1
        mov rghi, rg2
      %endif
        instruct rg3
   %endrep

%ELIFIDNI tmode, M         ; measure throughput with 1 memory operand

   %rep 100
      %if regsize == 8
        mov eax, ebx        ; avoid partial register dependence
        nop
      %elif regsize == 16
        mov eax, ebx
        mov edx, ecx
      %else
        mov rglo, rg1
        mov rghi, rg2
      %endif
        instruct sizeptr [rsi]
   %endrep

%ELSE
   %error unknown testmode
%ENDIF

        dec ebp
        jnz Testloop1

%endmacro

; disable default test loops
%define repeat1 1
%define repeat2 1

