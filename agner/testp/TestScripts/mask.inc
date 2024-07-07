; rndtr.inc                                                           2016-10-05 Agner Fog
;
; Define test code to test latency and throughput of instructions involving mask registers
;
; (c) Copyright 2016 by Agner Fog. GNU General Public License www.gnu.org/licenses
;
; Parameters:
;
; instruct1: instruction to test
;
; instruct2: Auxiliary instruction for round trip test
;
; regsize:   Register or operand sizes for vector operands
;
; masksize:  Size of mask register. 0 for none, default = 16.
;
; nmask:     Value of mask register (signed decimal number)
;
; usezero:   Add zeroing option if 1, {z}
;
; numop1, numop2: Number of register or memory operands for each instruction, not including mask input. Default = 3
;
; numimm1, numimm2: Number of immediate operands for each instruction. Default = 0
;
; immvalue1, immvalue2:  Value of immediate operands for each instruction. Default = 0
;
; tmode:      Test mode:
; MASKCMPL:   Latency of instruction with mask output and mask input, e.g. VCMPEQD k1{k1},zmm2,zmm3
; MASKCMPT:   Throughput of instruction with mask output, e.g. VPCMPEQD k1,zmm2,zmm3
; MASKBLENDL: Round trip latency of instruction with mask input and auxiliary instruction with mask output,
;             e.g. VPBLENDMD zmm0{k1},zmm1,zmm2 / VPCMPEQD k1,zmm3,zmm0
; MASKBLENDT: Throughput of instruction with mask input
; MASKL:      Latency of masked instruction, e.g. VPADDD zmm1{k1},zmm2,zmm1
; MASKT:      Throughput of masked instruction, e.g. VPADDD zmm1{k1},zmm2,zmm3
;
; -------------------------------------------------------------------------------------------------

; define undefined parameters

%ifndef repeat2
   %define repeat2 100
%endif

%ifndef instruct2
   %define instruct2
%endif

%ifndef regsize
   %define regsize 512
%endif
; register names are defined in TemplateB64.nasm

%ifndef masksize
   %define masksize 16
%endif

%ifndef nmask
   %define nmask -1
%endif

%ifndef usezero
   %define usezero 0
%endif

%ifndef numop1
   %define numop1 3
%endif

%ifndef numop2
   %define numop2 3
%endif

%ifndef numimm1
   %define numimm1 0
%endif

%ifndef numimm2
   %define numimm2 0
%endif

%ifndef immvalue1
   %define immvalue1 1
%endif

%ifndef immvalue2
   %define immvalue2 1
%endif


; initialization of registers
%macro testinit1 0
   %if masksize != 0
      mov rax, nmask
      %if masksize == 8
         kmovb k1, eax
      %elif masksize == 16
         kmovw k1, eax
      %elif masksize == 32
         kmovd k1, eax
      %elif masksize == 64
         kmovq k1, eax
      %else
         %error wrong mask size nmask
      %endif
   %endif
%endmacro

%macro testinit2 0
   xor eax, eax
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; test code for mode MASKCMPL
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
%ifidni tmode, MASKCMPL

   %macro testcode 0
      %if numimm1 == 1
         %define immoperands1 , immvalue1
      %else
         %define immoperands1
      %endif
      %if numop1 == 2
		 instruct1 k1{k1}, reg0 immoperands1
      %elif numop1 == 3
	     instruct1 k1{k1}, reg0, reg1 immoperands1
      %else
	     %error wrong number of operands numop1
	  %endif
   %endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; test code for mode MASKCMPT
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
%elifidni tmode, MASKCMPT

   %macro testcode 0
      %if numimm1 == 1
         %define immoperands1 , immvalue1
      %else
         %define immoperands1
      %endif
      %if masksize == 0
         %define usemask
      %else
         %define usemask {k1}
      %endif
      %if numop1 == 2
		 instruct1 k1 usemask, reg0 immoperands1
      %elif numop1 == 3
	     instruct1 k1 usemask, reg0, reg1 immoperands1
      %else
	     %error wrong number of operands numop1
	  %endif
   %endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; test code for mode MASKBLENDL
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
%elifidni tmode, MASKBLENDL

   %macro testcode 0
      %if numimm1 == 1
         %define immoperands1 , immvalue1
      %else
         %define immoperands1
      %endif
      %if numimm2 == 1
         %define immoperands2 , immvalue2
      %else
         %define immoperands2
      %endif
      %if usezero == 1
         %define usez  {z}
      %else
         %define usez
      %endif
      %if masksize == 0
         %define usemask
      %else
         %define usemask {k1}
      %endif
      
      %if numop1 == 2
		 instruct1 reg0 {k2} usez, reg1 immoperands1
      %elif numop1 == 3
		 instruct1 reg0 {k2} usez, reg1, reg2 immoperands1
      %else
	     %error wrong number of operands numop1
	  %endif
      %if numop2 == 2
		 instruct2 k2 usemask, reg0 immoperands2
      %elif numop2 == 3
		 instruct2 k2 usemask, reg3, reg0 immoperands2
      %else
	     %error wrong number of operands numop2
	  %endif

   %endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; test code for mode MASKBLENDT
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
%elifidni tmode, MASKBLENDT

   %macro testcode 0
      %if numimm1 == 1
         %define immoperands1 , immvalue1
      %else
         %define immoperands1
      %endif
      %if usezero == 1
         %define usez  {z}
      %else
         %define usez
      %endif      
      %if numop1 == 2
		 instruct1 reg0 {k1} usez, reg1 immoperands1
      %elif numop1 == 3
		 instruct1 reg0 {k1} usez, reg1, reg2 immoperands1
      %else
	     %error wrong number of operands numop1
	  %endif
   %endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; test code for mode MASKL
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
%elifidni tmode, MASKL

   %macro testcode 0
      %if numimm1 == 1
         %define immoperands1 , immvalue1
      %else
         %define immoperands1
      %endif
      %if usezero == 1
         %define usez  {z}
      %else
         %define usez
      %endif
      %if masksize == 0
         %define usemask
      %else
         %define usemask {k1}
      %endif
      
      %if numop1 == 2
		 instruct1 reg0 usemask usez, reg0 immoperands1
      %elif numop1 == 3
		 instruct1 reg0 usemask usez, reg1, reg0 immoperands1
      %else
	     %error wrong number of operands numop1
	  %endif

   %endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; test code for mode MASKT
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
%elifidni tmode, MASKT

   %macro testcode 0
      %if numimm1 == 1
         %define immoperands1 , immvalue1
      %else
         %define immoperands1
      %endif
      %if usezero == 1
         %define usez  {z}
      %else
         %define usez
      %endif
      %if masksize == 0
         %define usemask
      %else
         %define usemask {k1}
      %endif
      
      %if numop1 == 2
		 instruct1 reg0 usemask usez, reg1 immoperands1
      %elif numop1 == 3
         instruct1 reg0 usemask usez, reg1, reg2 immoperands1
      %else
	     %error wrong number of operands numop1
	  %endif

   %endmacro

%else
   %error unknown test mode tmode
%endif
