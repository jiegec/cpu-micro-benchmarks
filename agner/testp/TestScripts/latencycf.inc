; latencycf.inc                                              2016-10-23 Agner Fog
; Define test code for measuring latency of miscellaneous instructions with carry flag output
; (c) Copyright 2016 by Agner Fog. GNU General Public License www.gnu.org/licenses

; Parameters:
;
; instruct:   Instruction to test
;
; regsize:    Register size: 8, 16, 32, 64, 128, 256, 512.
;
; regtype:    Register type: v = vector register 128 bits and bigger, k = mask register. 
;
; numop:      Number of register operands, not including flags register
;
; numimm:     Number of immediate operands. Default = 0
;
; immvalue:   Value of first immediate operand. Default = 0


; initialize eax
%macro testinit2 0
   xor eax, eax
%endmacro

; Define specific test code for each register type

%ifidni regtype, v

   %macro testcode 0
      %if numop == 2
         instruct reg1, reg1
         setb al
         %if regsize == 128
            movd xmm1, eax
         %else
            vmovd xmm1, eax
         %endif
      %else
         %error unsupported number of operands numop
      %endif
   %endmacro

%elifidni regtype, k

   %macro testcode 0
      %if numop == 2
         instruct k1,k1
         setb al
         kmovw k1,eax
      %else
         %error unsupported number of operands numop
      %endif
   %endmacro

%else

   %error unknown register type regtype

%endif

