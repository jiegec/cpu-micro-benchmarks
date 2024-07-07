;----------------------------------------------------------------------------
;                       shift.inc                           2012-01-26 Agner Fog
;
;            PMC Test program for shift and rotate instructions
;                           YASM syntax
;
; The following macros can be defined on the command line or in include files:
; 
; instruct:     The name of a single instruction to test
; 
; regsize:      Register size: 8, 16, 32, 64
; 
; cntop:        Count operand: must be integer constant or cl
;
; tmode:        L:  Latency
;               T:  Throughput
;               M:  Throughput with memory operand
;
; (c) Copyright 2012 by Agner Fog. GNU General Public License www.gnu.org/licenses
;-----------------------------------------------------------------------------
%ifndef tmode
   %define tmode  L          ; default: measure latency
%endif

%ifndef cntop
   %define cntop   1         ; default count operand = 1
%endif


;##############################################################################
;#
;#                 Test code macro
;#
;##############################################################################


%macro blockports 0    ; currently unused
%endmacro  


; main testcode macro
%macro testcode 0

%ifidni instruct, shld    ; define second operand for shld and shrd only
   %if modesize == 64
      %define op2 reg7,
   %else
      %define op2 reg6,
   %endif
%elifidni instruct, shrd
   %if modesize == 64
      %define op2 reg7,
   %else
      %define op2 reg6,
   %endif
%else
   %define op2 
%endif

; start loop
        mov ecx, 5
        mov ebp,100
        align 32
Testloop1:        

%IFIDNI tmode, L         ; measure latency

   %rep 100 
        instruct reg0, op2 cntop
   %endrep

%ELIFIDNI tmode, T         ; measure throughput with register operands

   %rep 25
        instruct reg0, op2 cntop
        blockports
        instruct reg1, op2 cntop
        blockports
        instruct reg3, op2 cntop      ; avoid ecx
        blockports
        instruct reg4, op2 cntop
        blockports
   %endrep

%ELIFIDNI tmode, M         ; measure throughput with memory operand

   %rep 25
        instruct sizeptr [rsi], op2 cntop
        blockports
        instruct sizeptr [rsi+regsize], op2 cntop
        blockports
        instruct sizeptr [rsi+regsize*2], op2 cntop
        blockports
        instruct sizeptr [rsi+regsize*3], op2 cntop
        blockports
   %endrep

%ELSE
   %error unknown testmode
%ENDIF

        dec ebp
        jnz Testloop1

%endmacro ; testcode

; disable default test loops
%define repeat1 1
%define repeat2 1

