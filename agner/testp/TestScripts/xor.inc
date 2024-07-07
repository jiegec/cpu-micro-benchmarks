; xor.inc                                                                 2012-02-19 AgF
; Run PMCTest for various instructions
; to test if they are independent of input operands
; (c) 2012 by Agner Fog. GNU General Public License www.gnu.org/licenses

; Parameters:
;
; instruct:   Instruction to test
;
; instruct2:  Another instruction to include in dependency chain
;
; regsize:    Register size: 8, 16, 32, 64, 128, 256, (or 65 = mmx). Default = 32
;
; numop:      Number of register operands (2-4)
;
; numimm:     Number of immediate operands to instruct (0 - 2). Default = 0
;
; immvalue:   Value of first immediate operand. Default = 0
;
; clearflags: 1 = clear carry flag before instruction
;
; readflags:  1 = read carry flag after instruction
;             2 = read zero flag after instruction
;
; tmode:      Test mode:
;             1:   measure latency with different registers
;             2:   measure latency with same register twice
;             3:   measure latency with different register alternating with instruct2
;             4:   measure latency with same register twice alternating with instruct2

%if numop == 2
   %define operandssame reg0,reg0
   %define operandsdiff reg0,reg1
%elif numop == 3
   %define operandssame reg0,reg0,reg0
   %define operandsdiff reg0,reg0,reg1
%elif numop == 4
   %define operandssame reg0,reg0,reg0,reg0
   %define operandsdiff reg0,reg0,reg1,reg2
%else
   %error unknown number of operands
%endif

%ifdef clearflags
   %if clearflags > 0
      %macro clearbefore 0
         clc
      %endmacro
   %else
      %macro clearbefore 0
      %endmacro
   %endif
%else
   %macro clearbefore 0
   %endmacro
%endif

%ifdef readflags
   %if readflags == 1              ; read carry flag after instruction
      %macro readflagafter 0
         sbb eax,eax
      %endmacro
   %elif readflags == 2            ; read zero flag after instruction
      %macro readflagafter 0
         sete al
         movzx eax,al
      %endmacro
   %else
      %macro readflagafter 0
      %endmacro
   %endif
%else
   %macro readflagafter 0
   %endmacro
%endif


; main testcode macro
%macro testcode 0

;   %if regsize <= 64  ; reset registers
;      xor eax,eax
;      xor ebx,ebx
;      xor ecx,ecx
;      xor edx,edx
;      xor edi,edi
;   %endif

   %if tmode == 1           ; measure latency with different registers
      %rep 100
         clearbefore
         instruct operandsdiff immoperands1
         readflagafter
      %endrep
   %elif tmode == 2         ; measure latency with same register twice
      %rep 100
         clearbefore
         instruct operandssame immoperands1
         readflagafter
      %endrep
   %elif tmode == 3         ; measure latency with different registers alternating with instruct2
      %rep 100
         clearbefore
         instruct operandsdiff immoperands1
         readflagafter
         instruct2 operandssame
      %endrep
   %elif tmode == 4         ; measure latency with same register twice alternating with instruct2
      %rep 100
         clearbefore
         instruct operandssame immoperands1
         readflagafter
         instruct2 operandssame
      %endrep
   %else
      %error unknown test mode tmode
   %endif

%endmacro

; disable default test loops
%define repeat1 100
%define repeat2 1

