; evex.inc                                              2022-05-11 Agner Fog
;
; Test instructions with EVEX prefix that are not defined by the NASM assembler.
; These instructions are hard coded. The code is defined in the options field as
; evex=0xMPWCC where:
; M  is the value of the mmm bits, where 1 = 0F, 2 = 0F 38, 3 = 0F 3A, 4-5 for AVX512_FP16
; P  is the pp bits, where 0 = none, 1 = 66, 2 = F3, 3 = F2
; W  is the mask in bits 4-6, z in bit 2, b in bit 1, W in bit 0
; CC is the opcode byte
;
; Define test code to test latency and throughput for instructions with 2-3 register operands
; and optionally one immediate operand, 64 bit mode.
; (c) Copyright 2013-2022 by Agner Fog. GNU General Public License www.gnu.org/licenses
;
; Parameters:
;
; instruct:   Name of instruction to test
;
; evex:       Code details as defined above
;
; regsize:    Register size: 128, 256, 512. Default = 128
;
; regtype:    Register type: r = general purpose register, 
;             v = vector register 128 bits and bigger, k = mask register. 
;             Default is r for regsize <= 64, v for regsize >= 128
;
; numop:      Number of register operands 2 - 3
;
; numimm:     Number of immediate operands (0 - 1). Default = 0
;
; immvalue:   Value of first immediate operand. Default = 0
;
; tmode1:     Test mode:
;             L:   measure latency
;             T:   measure throughput
;             L0:  measure latency from operand 0
;             T0:  measure throughput with register operands where op0 is both source and destination
;             S:   measure throughput with eight different destination registers, numop = 2
;             Test modes with memory operands are not supported
;
; regval0:    Initial value of source register reg0, integer or floating point number, default = 0
;
; regval1:    Initial value of source register reg1, integer or floating point number, default = 0

; replace loops in templateb64.nasm:
%ifndef repeat1
   %define repeat1 100
%endif
%if repeat1 > 1
   %assign loop1count repeat1
%else
   %ifndef loop1count
      %assign loop1count 100
   %endif
%endif
%ifidni tmode1, S        ; needs total count divisible by 8
   %assign loop1count 50
%endif

; disable default test loops
%define noloops 1  ;      Define this because repeat1 and repeat2 loops are contained in testcode rather than in templateBxx.nasm

%ifndef immvalue
   %define immvalue 0
%endif
%ifnnum immvalue
   %define immvalue 0
%endif

%ifdef regtype
   %ifidni regtype, k
      %define numregs 8
   %elif modesize == 32
      %define numregs 8
   %elifidni regtype v
      %define numregs 32
   %else
      %define numregs 16
   %endif
%else
   %define numregs 16
%endif

; initialization for regval0 and regval1
%ifndef elementsize
   %define elementsize 16
%endif
%ifempty elementsize
   %define elementsize 16
%endif

%ifndef elementsize
   %define regsize 128
%endif
%if regsize == 128
   %define LL 0
%elif regsize == 256
   %define LL 1
%elif regsize == 512
   %define LL 2
%else
   ; %error unknown register size regsize
   ; problem here if we have both k register and vector register
   %define LL 2
%endif


%ifdef regval0
   %ifnempty regval0
      %ifndef regval1
         %define regval1 regval0
      %endif
      %ifempty regval1
         %define regval1 regval0
      %endif
      %macro testdata 0
         %if elementsize == 8
            %define elementdef DB
         %elif elementsize == 16
            %define elementdef DW
         %elif elementsize == 32
            %define elementdef DD
         %elif elementsize == 64
            %define elementdef DQ
         %else
            %error unsupported element size elementsize
         %endif
         %rep regsize/elementsize*0x100
            elementdef regval0
         %endrep
         %rep regsize/elementsize*0x10
            elementdef regval1
         %endrep
      %endmacro

      %macro testinit1 0
         lea rax, [UserData]
         lea rbx, [rax+regsize/elementsize*0x100]
         %if regsize == 128
            movdqa reg0, [rax]
            movdqa reg1, [rbx]
            movdqa reg2, [rbx]
            movdqa reg3, [rbx]
            movdqa reg4, [rbx]
         %elif regsize == 256
            vmovdqa reg0, [rax]
            vmovdqa reg1, [rbx]
            vmovdqa reg2, [rbx]
            vmovdqa reg3, [rbx]
            vmovdqa reg4, [rbx]
         %else
            vmovdqa32 reg0, [rax]
            vmovdqa32 reg1, [rbx]
            vmovdqa32 reg2, [rbx]
            vmovdqa32 reg3, [rbx]
            vmovdqa32 reg4, [rbx]
         %endif
      %endmacro
   %endif
%endif

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; macro to hardcode EVEX instruction
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Code EVEX instruction not supported by NASM
; no memory operand
; parameters:
; 1: mmm = mmm bits: 1 = 0F, 2 = 0F 38, 3 = 0F 3A, 4-5 for AVX512_FP16
; 2: pp = pp bits: 0 = none, 1 = 66, 2 = F3, 3 = F2
; 3: L'L: 0 = 128, 1 = 256, 2 = 512
; 4: mask,0,z,b,W
; 5: Opcode
; 6: reg1 R bits
; 7: reg2 vvvvv bits
; 8: reg3 B bits
; 9: immediate byte if mmm = 1
%macro EVEXINSTR 9
DB 0x62
DB %1 | (%6 >> 4 & 1 ^ 1) << 4 | (%8 >> 3 & 1 ^ 1) << 5 | (%8 >> 4 & 1 ^ 1) << 6 | (%6 >> 3 & 1 ^ 1) << 7  ; R X B R' mmmm
DB (%2 & 3) | 1 << 2 | (%7 & 0xF ^ 0xF) << 3 | (%4 & 1) << 7  ; W vvvv 1 pp
DB (%4 >> 4 & 7) | (%7 >> 4 & 1 ^ 1) << 3 | ((%4 & 2) >> 1 & 1) << 4 | (%3 & 3) << 5 | (%4 >> 2 & 1) << 7  ; z LL b V' aaa

DB %5
DB (%8 & 7) | (%6 & 7) << 3 | 0xC0  ; mod reg rm
%if %1 == 3
   DB %9
%endif
%endmacro 

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; macro to implement instruction
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; The parameters are 3 register numbers. 
; evex, regsize, and immvalue are implicit parameters
; evex=0xMPWCC where:
; M  is the value of the mmm bits, where 1 = 0F, 2 = 0F 38, 3 = 0F 3A, 4-5 for AVX512_FP16
; P  is the pp bits, where 0 = none, 1 = 66, 2 = F3, 3 = F2
; W  is the mask in bits 4-6, z in bit 2, b in bit 1, W in bit 0
; CC is the opcode byte

%macro INSTRUCTCODE 3
EVEXINSTR (evex >> 16 & 0xF), (evex >> 12 & 3), LL, (evex >> 8 & 3), (evex & 0xFF), %1, %2, %3, immvalue
%endmacro 


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; main testcode macro
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%macro testcode 0

   %if regsize <= 64  ; reset registers
      xor eax,eax
      xor ebx,ebx
      xor ecx,ecx
      xor edx,edx
      xor edi,edi
   %endif

   %if modesize == 32
      %define loopcounter ebp
   %else
      %define loopcounter r12d
   %endif

   ; test loop 1
   mov loopcounter, loop1count
   align 32
   Testloop1:

   %if numop == 2  ; 2 register operands
      %ifidni tmode1, L         ; measure latency
         %rep 50
            INSTRUCTCODE 1, 0, 2
            INSTRUCTCODE 2, 0, 1
         %endrep
      %elifidni tmode1, T         ; measure throughput with register operands
         %if numregs < 11        ; mmx and k registers
            %rep 20
               INSTRUCTCODE 2, 0, 1
               INSTRUCTCODE 3, 0, 1
               INSTRUCTCODE 4, 0, 1
               INSTRUCTCODE 5, 0, 1
               INSTRUCTCODE 6, 0, 1
            %endrep
         %else
            %rep 10
               INSTRUCTCODE 2,  0, 1
               INSTRUCTCODE 3,  0, 1
               INSTRUCTCODE 4,  0, 1
               INSTRUCTCODE 5,  0, 1
               INSTRUCTCODE 6,  0, 1
               INSTRUCTCODE 7,  0, 1
               INSTRUCTCODE 8,  0, 1
               INSTRUCTCODE 9,  0, 1
               INSTRUCTCODE 10, 0, 1
               INSTRUCTCODE 11, 0, 1
            %endrep
         %endif

      %elifidni tmode1, S       ; measure throughput with eight different destination registers
         %rep 25
            INSTRUCTCODE 2, 0, 1
            INSTRUCTCODE 3, 0, 1
            INSTRUCTCODE 4, 0, 1
            INSTRUCTCODE 5, 0, 1
            INSTRUCTCODE 6, 0, 1
            INSTRUCTCODE 7, 0, 1
            INSTRUCTCODE 8, 0, 1
            INSTRUCTCODE 9, 0, 1
         %endrep

      %else
         %error unknown tmode1
      %endif

   %elif numop == 3  ; 3 register operands (AVX/XOP/FMA3)
   
      %ifidni tmode1, L           ; measure latency
         %rep 50
            INSTRUCTCODE 0, 1, 2
            INSTRUCTCODE 2, 1, 0
         %endrep
      %elifidni tmode1, L0        ; measure latency from operand 0
         %rep 50
            INSTRUCTCODE 0, 1, 2
            INSTRUCTCODE 0, 4, 5
         %endrep
      %elifidni tmode1, T         ; measure throughput with register operands
         %rep 50
            INSTRUCTCODE 5, 2, 0
            INSTRUCTCODE 6, 4, 1
         %endrep
      %elifidni tmode1, T0        ; measure throughput with register operands where op0 is both source and destination
         %rep 10
            INSTRUCTCODE 2,  1, 0
            INSTRUCTCODE 3,  1, 0
            INSTRUCTCODE 4,  1, 0
            INSTRUCTCODE 5,  1, 0
            INSTRUCTCODE 6,  1, 0
            INSTRUCTCODE 7,  1, 0
            INSTRUCTCODE 8,  1, 0
            INSTRUCTCODE 9,  1, 0
            INSTRUCTCODE 10, 1, 0
            INSTRUCTCODE 11, 1, 0
         %endrep
      %else
         %error unknown tmode1
      %endif

   %else
      %error unsupported numop
   %endif

   dec loopcounter
   jnz Testloop1   ; loop
%endmacro ; testcode
