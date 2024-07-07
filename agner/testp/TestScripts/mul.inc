;----------------------------------------------------------------------------
;                       mul.inc                  2012-01-26 Agner Fog
;
;            PMC Test program for integer multiplication instructions
; The following macros can be defined on the command line or in include files:
; 
; instruct:     The name of a single instruction to test
; 
; regsize:      Register size: 8, 16, 32, 64, 128, 256, (or 65 = mmx)
; 
; numop:        Number of operands: 1, 2 or 3
;
; tmode:        L:  Latency
;               T:  Throughput
;               M:  Throughput with memory operand
;
; (c) Copyright 2012 by Agner Fog. GNU General Public License www.gnu.org/licenses
;-----------------------------------------------------------------------------
; Define any undefined macros

%ifndef tmode
   %define tmode  L          ; default: measure latency
%endif

%ifndef regsize              ; default: define registers as 32 bit
   %define regsize   32
%endif

%ifndef numop
   %define numop   2         ; default number of operands
%endif


;##############################################################################
;#
;#                 Test code macro
;#
;##############################################################################


; main testcode macro
%macro testcode 0

        mov ebp,100
        align 32
Testloop1:

%IF numop == 1           ; single operand        

%IFIDNI tmode, L         ; measure latency, 1 operand

   %rep 100 
        instruct reg4
   %endrep

%ELIFIDNI tmode, T         ; measure throughput, 1 operand

   %rep 100
        xor eax, eax 
        instruct reg4
   %endrep

%ELIFIDNI tmode, M         ; measure throughput with 1 memory operand

   %rep 100
        xor eax,eax
        instruct sizeptr [rsi]
   %endrep

%ELSE
   %error unknown testmode
%ENDIF

%ELIF numop == 2           ; two operands

%IFIDNI tmode, L         ; measure latency, 2 operands

   %rep 100 
        instruct reg1, reg0
   %endrep

%ELIFIDNI tmode, T         ; measure throughput, 2 operands

   %rep 25
        instruct reg1, reg0
        instruct reg2, reg0
        instruct reg3, reg0
        instruct reg4, reg0
   %endrep

%ELIFIDNI tmode, M         ; measure throughput with 2 operands, including 1 memory operand

   %rep 25
        instruct reg1, sizeptr [rsi]
        instruct reg2, sizeptr [rsi+regsize]
        instruct reg3, sizeptr [rsi+regsize*2]
        instruct reg4, sizeptr [rsi+regsize*3]
   %endrep

%ELSE
   %error unknown testmode
%ENDIF


%ELIF numop == 3           ; three operands

%IFIDNI tmode, L         ; measure latency, 3 operands

   %rep 100 
        instruct reg0, reg0, 55
   %endrep

%ELIFIDNI tmode, T         ; measure throughput, 3 operands

   %rep 25
        instruct reg1, reg0, 55
        instruct reg2, reg0, 66
        instruct reg3, reg0, 77
        instruct reg4, reg0, 88
   %endrep

%ELIFIDNI tmode, M         ; measure throughput with 3 operands, including 1 memory operand

   %rep 25
        instruct reg1, sizeptr [rsi], 55
        instruct reg2, sizeptr [rsi+regsize], 66
        instruct reg3, sizeptr [rsi+regsize*2], 77
        instruct reg4, sizeptr [rsi+regsize*3], 88
   %endrep

%ELSE
   %error unknown testmode
%ENDIF


%ELSE
   %error wrong number of operands
%ENDIF


        dec ebp
        jnz Testloop1
%endmacro

; disable default test loops
%define repeat1 1
%define repeat2 1

