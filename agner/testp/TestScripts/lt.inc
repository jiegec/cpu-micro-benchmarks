; lt.inc                                              2022-05-11 Agner Fog
;
; Define universal test code to test latency and throughput for instructions with 0-4 register operands
; and optionally one immediate operand, 64 bit mode.
; (c) Copyright 2013-2022 by Agner Fog. GNU General Public License www.gnu.org/licenses
;
; Parameters:
;
; instruct:   Instruction to test
;
; regsize:    Register size: 8, 16, 32, 64, 128, 256, 512. Default = 32
;             (Legacy code has regsize=65 indicating mmx register)
;
; regtype:    Register type: r = general purpose register, h = high 8-bit register,
;             v = vector register 128 bits and bigger, m = mmx register, k = mask register. 
;             Default is r for regsize <= 64, v for regsize >= 128
;
; numop:      Number of register operands (0 - 2, 3 if regsize >= 256)
;
; numimm:     Number of immediate operands (0 - 2). Default = 0
;
; immvalue:   Value of first immediate operand. Default = 0
;
; tmode:      Test mode:
;             L:   measure latency
;             T:   measure throughput
;             M:   measure throughput with memory source operand
;             MR:  measure throughput with memory destination operand (only for numop = 2)
;             LMR: measure latency with memory destination operand (only for numop = 1 or 2)
;             M3:  measure throughput with memory source in operand 3 (only for numop = 4)
;             L2:  measure latency from operand 2 (only for numop = 4)
;             L4:  measure latency from operand 4 (only for numop = 4)
;             S:   measure throughput with eight different destination registers (only for numop = 2)
;             A:   clear eax (only for numop = 0)
;             D:   clear eax and edx (only for numop = 0)
;
; blockp:     Insert instructions that block a particular port or pipe when measuring throughput
;             Possible values for Intel: 0, 1, 5
;             Possible values for AMD:   0, 1
;
; cpubrand:   CPU brand: Intel, AMD, VIA
;
; elementsize: Size of vector elements in bits. Only required if regval0 or regval1 specified
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
%ifidni tmode, S        ; needs total count divisible by 8
   %assign loop1count 50
%endif

; disable default test loops
%define noloops 1  ;      Define this because repeat1 and repeat2 loops are contained in testcode rather than in templateBxx.nasm


; Macro that blocks port 0, 1 or 5 on Intel Sandy Bridge or later
; Blocks port 0, 1 or none(9) on Intel silvermont
%macro blockportIntel 0
   %ifdef blockp
      %if imodel==0x37 || imodel==0x4A || imodel==0x4D
         ; Silvermont
         %if blockp   == 0
            ; Insert extra instruction that blocks port 0
            movsx r11,r11w
         %elif blockp == 1
            ; Insert extra instruction that blocks port 1
            lea r11d,[r11+r11]
         %elif blockp == 9
            ; Insert NOP to see if uop is pairable
            nop
         %endif
      %else
         ; other Intel (Sandy Bridge and later)
         %if blockp   == 0
            ; Insert extra instruction that blocks port 0
            pmullw mm2,mm3
            %define  USEAVX  1
         %elif blockp == 1
            ; Insert extra instruction that blocks port 1
            ; vaddss xmm9,xmm10,xmm10
            lea r11, [ThreadData]
            ; bsf r11d,r10d
            %define  USEAVX  1
         %elif blockp == 5
            ; Insert extra instruction that blocks port 5
            jo $+2
         %endif
      %endif
   %endif
%endmacro

; Macro that blocks integer unit EX0 or EX1 on AMD Bulldozer or later
%macro blockportAMD 0
   %ifdef blockp
      %if blockp   == 0
         ; Insert extra instruction that blocks integer pipe 0
         lzcnt  r8d, r9d    ; EX0, Latency 2, throughput 1/2
      %elif blockp == 1
         ; Insert extra instruction that blocks integer pipe 1
         popcnt r8d, r9d    ; EX1, Latency 4, throughput 1/2
      %endif
   %endif
%endmacro

%macro blockEven 0   ; block a port, on even iterations
   %ifdef cpubrand
      %ifidni   cpubrand, Intel
         blockportIntel
      %elifidni cpubrand, AMD
         blockportAMD
      %endif
   %endif
%endmacro

%macro blockOdd 0   ; block a port, on odd iterations
   %ifdef cpubrand
      %ifidni   cpubrand, Intel
         blockportIntel
      %elifidni cpubrand, AMD
         ; nothing because blockEven has throughput 1/2
      %endif
   %endif
%endmacro

; initialization for regval0 and regval1
%ifndef elementsize
   %define elementsize 32
%endif
%ifempty elementsize
   %define elementsize 32
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

   %if numop == 0  ; 0 register operands

      %ifidni tmode, L           ; measure latency or throughput
         %rep 50
            instruct immoperands0
            blockEven
            instruct immoperands0
            blockOdd
         %endrep
      %elifidni tmode, T         ; measure latency or throughput
         %rep 50
            instruct immoperands0
            blockEven
            instruct immoperands0
            blockOdd
         %endrep
      %elifidni tmode, A         ; clear eax
         %rep 50
            xor eax,eax
            instruct immoperands0
            blockEven
            xor eax,eax
            instruct immoperands0
            blockOdd
         %endrep
      %elifidni tmode,D         ; clear eax and edx
         %rep 50
            xor eax,eax
            xor edx,edx
            instruct immoperands0
            blockEven
            xor eax,eax
            xor edx,edx
            instruct immoperands0
            blockOdd
         %endrep
      %else
         %error unknown testmode
      %endif

   %elif numop == 1  ; 1 register operand

      %ifidni tmode, L           ; measure latency
         %rep 100 
            instruct reg0 immoperands1
         %endrep
      %elifidni tmode, T         ; measure throughput with register operands
         %rep 25
            instruct reg0 immoperands1
            blockEven
            instruct reg1 immoperands1
            blockOdd
            instruct reg2 immoperands1
            blockEven
            instruct reg3 immoperands1
            blockOdd
         %endrep
      %elifidni tmode, S         ; measure throughput with 8 different register operands
         %rep 25
            instruct reg0 immoperands1
            instruct reg1 immoperands1
            instruct reg2 immoperands1
            instruct reg3 immoperands1
            instruct reg4 immoperands1
            instruct reg5 immoperands1
            instruct reg6 immoperands1
            instruct reg7 immoperands1
         %endrep
      %elifidni tmode, M         ; measure throughput with memory operand
         %rep 10
            instruct sizeptr [rsi] immoperands1
            blockEven
            instruct sizeptr [rsi+regsize/8] immoperands1
            blockOdd
            instruct sizeptr [rsi+regsize/8*2] immoperands1
            blockEven
            instruct sizeptr [rsi+regsize/8*3] immoperands1
            blockOdd
            instruct sizeptr [rsi+regsize/8*4] immoperands1
            blockEven
            instruct sizeptr [rsi+regsize/8*5] immoperands1
            blockOdd
            instruct sizeptr [rsi+regsize/8*6] immoperands1
            blockEven
            instruct sizeptr [rsi+regsize/8*7] immoperands1
            blockOdd
            instruct sizeptr [rsi+regsize/8*8] immoperands1
            blockEven
            instruct sizeptr [rsi+regsize/8*9] immoperands1
            blockOdd
         %endrep
      %elifidni tmode, LMR        ; measure latency with memory destination operand
         %rep 50
            instruct sizeptr [rsi] immoperands1
            blockEven
            instruct sizeptr [rsi] immoperands1
            blockOdd
         %endrep
      %else
         %error unknown testmode
      %endif

   %elif numop == 2  ; 2 register operands
      %ifidni tmode, L         ; measure latency
         %rep 50 
            instruct reg1, reg0 immoperands1
            instruct reg0, reg1 immoperands1
         %endrep
      %elifidni tmode, T         ; measure throughput with register operands
         %if numregs < 11        ; mmx and k registers
            %rep 20
               instruct reg1, reg0 immoperands1
               blockEven
               instruct reg2, reg0 immoperands1
               blockOdd
               instruct reg3, reg0 immoperands1
               blockEven
               instruct reg4, reg0 immoperands1
               blockOdd
               instruct reg5, reg0 immoperands1
               blockOdd
            %endrep
         %else
            %rep 10
               instruct reg1, reg0 immoperands1
               blockEven
               instruct reg2, reg0 immoperands1
               blockOdd
               instruct reg3, reg0 immoperands1
               blockEven
               instruct reg4, reg0 immoperands1
               blockOdd
               instruct reg5, reg0 immoperands1
               blockEven
               instruct reg6, reg0 immoperands1
               blockOdd
               instruct reg7, reg0 immoperands1
               blockEven
               instruct reg8, reg0 immoperands1
               blockOdd
               instruct reg9, reg0 immoperands1
               blockEven
               instruct reg10, reg0 immoperands1
               blockOdd               
            %endrep
         %endif

      %elifidni tmode, S       ; measure throughput with eight different destination registers
         %rep 25
            instruct reg1, reg0 immoperands1
            instruct reg2, reg0 immoperands1
            instruct reg3, reg0 immoperands1
            instruct reg4, reg0 immoperands1
            instruct reg5, reg0 immoperands1
            instruct reg6, reg0 immoperands1
            instruct reg7, reg0 immoperands1
            instruct reg8, reg0 immoperands1
         %endrep
         
      %elifidni tmode, M         ; measure throughput with memory source operand
         %if numregs >= 11
         %rep 10
            instruct reg0, [rsi] immoperands1
            blockEven
            instruct reg1, [rsi+regsize/8] immoperands1
            blockOdd
            instruct reg2, [rsi+regsize/8*2] immoperands1
            blockEven
            instruct reg3, [rsi+regsize/8*3] immoperands1
            blockOdd
            instruct reg4, [rsi+regsize/8*4] immoperands1
            blockEven
            ; avoid reg5 = rsi if registersize = 32/64
            instruct reg6, [rsi] immoperands1
            blockOdd
            instruct reg7, [rsi+regsize/8] immoperands1
            blockEven
            instruct reg8, [rsi+regsize/8*2] immoperands1
            blockOdd
            instruct reg9, [rsi+regsize/8*3] immoperands1
            blockEven
            instruct reg10, [rsi+regsize/8*4] immoperands1
            blockOdd
         %endrep         
         %else  ; 32 bit mode. reg8-10 not defined
         %rep 20
            instruct reg0, [rsi] immoperands1
            blockEven
            instruct reg1, [rsi+regsize/8] immoperands1
            blockOdd
            instruct reg2, [rsi+regsize/8*2] immoperands1
            blockEven
            instruct reg3, [rsi+regsize/8*3] immoperands1
            blockOdd
            instruct reg4, [rsi+regsize/8*4] immoperands1  ; avoid reg5 = rsi
            blockOdd
         %endrep         
         %endif
         
      %elifidni tmode, MR        ; measure throughput with memory destination operand
         %rep 10
            instruct [rsi],             reg0
            blockEven
            instruct [rsi+regsize/8],   reg1
            blockOdd
            instruct [rsi+regsize/8*2], reg2
            blockEven
            instruct [rsi+regsize/8*3], reg3
            blockOdd
            instruct [rsi+regsize/8*4], reg4
            blockEven
            instruct [rsi+regsize/8*5], reg0
            blockOdd
            instruct [rsi+regsize/8*6], reg1
            blockEven
            instruct [rsi+regsize/8*7], reg2
            blockOdd
            instruct [rsi+regsize/8*8], reg3
            blockEven
            instruct [rsi+regsize/8*9], reg4
            blockOdd
         %endrep
      %elifidni tmode, LMR        ; measure latency with memory destination operand
         %rep 50
            instruct [rsi], reg0
            blockEven
            instruct [rsi], reg0
            blockOdd
         %endrep

      %else
         %error unknown testmode
      %endif

   %elif numop == 3  ; 3 register operands (AVX/XOP/FMA3)
   
      %ifidni tmode, L         ; measure latency
         %rep 50 
            instruct reg2, reg1, reg0 immoperands1
            instruct reg0, reg1, reg2 immoperands1
         %endrep
      %elifidni tmode, L0        ; measure latency from operand 0
         %rep 50 
            instruct reg0, reg1, reg2 immoperands1
            instruct reg0, reg4, reg5 immoperands1
         %endrep
      %elifidni tmode, T         ; measure throughput with register operands
         %rep 50
            instruct reg5, reg2, reg0 immoperands1
            blockEven
            ; avoid reg3 = edx, which is implicit operand in mulx instruction
            instruct reg6, reg4, reg1 immoperands1
            blockOdd
         %endrep
      %elifidni tmode, T0        ; measure throughput with register operands where op0 is both source and destination
         %if numregs >= 12
         %rep 10
            instruct reg2,  reg1, reg0 immoperands1
            blockEven
            instruct reg3,  reg1, reg0 immoperands1
            blockOdd
            instruct reg4,  reg1, reg0 immoperands1
            blockEven
            instruct reg5,  reg1, reg0 immoperands1
            blockOdd
            instruct reg6,  reg1, reg0 immoperands1
            blockEven
            instruct reg7,  reg1, reg0 immoperands1
            blockOdd
            instruct reg8,  reg1, reg0 immoperands1
            blockEven
            instruct reg9,  reg1, reg0 immoperands1
            blockOdd
            instruct reg10, reg1, reg0 immoperands1
            blockEven
            instruct reg11, reg1, reg0 immoperands1
            blockOdd
         %endrep
         %else  ; 32 bit mode. reg8-11 not defined
         %rep 10
            instruct reg2,  reg1, reg0 immoperands1
            blockEven
            instruct reg3,  reg1, reg0 immoperands1
            blockOdd
            instruct reg4,  reg1, reg0 immoperands1
            blockEven
            instruct reg5,  reg1, reg0 immoperands1
            blockOdd
            instruct reg6,  reg1, reg0 immoperands1
            blockEven
            instruct reg7,  reg1, reg0 immoperands1
            blockOdd
            instruct reg3,  reg1, reg0 immoperands1
            blockEven
            instruct reg4,  reg1, reg0 immoperands1
            blockOdd
            instruct reg5,  reg1, reg0 immoperands1
            blockEven       
            instruct reg6,  reg1, reg0 immoperands1
            blockOdd
         %endrep         
         %endif         
      %elifidni tmode, M         ; measure throughput with memory source operand
         %rep 50
            instruct reg0, reg1, [rsi] immoperands1
            blockEven
            instruct reg2, reg3, [rsi+regsize/8] immoperands1
            blockOdd
         %endrep
      %elifidni tmode, M2        ; measure throughput with memory as second operand
         %rep 25
            instruct reg0, [rsi], reg1 immoperands1
            blockEven
            instruct reg2, [rsi+regsize/8], reg3 immoperands1
            blockOdd
            instruct reg4, [rsi], reg1 immoperands1
            blockEven
            instruct reg6, [rsi+regsize/8], reg3 immoperands1
            blockOdd
         %endrep         
      %elifidni tmode, M0         ; measure throughput with memory source operand where op0 is both source and destination
         %rep 10
            instruct reg1, reg0, [rsi] immoperands1
            blockEven
            instruct reg2, reg0, [rsi+regsize/8] immoperands1
            blockOdd
            instruct reg3, reg0, [rsi+2*regsize/8] immoperands1
            blockEven
            instruct reg4, reg0, [rsi+3*regsize/8] immoperands1
            blockOdd
            instruct reg6, reg0, [rsi+4*regsize/8] immoperands1
            blockEven
            instruct reg7, reg0, [rsi+0*regsize/8] immoperands1
            blockOdd
            instruct reg8, reg0, [rsi+1*regsize/8] immoperands1
            blockEven
            instruct reg9, reg0, [rsi+2*regsize/8] immoperands1
            blockOdd
            instruct reg10, reg0, [rsi+3*regsize/8] immoperands1
            blockEven
            instruct reg11,reg0, [rsi+4*regsize/8] immoperands1            
         %endrep
      %else
         %error unknown testmode
      %endif

   %elif numop == 4  ; 4 register operands (AVX/XOP/FMA4)
      %ifidni tmode, L         ; measure latency
         %rep 50 
            instruct reg0, reg1, reg1, reg1 immoperands1
            instruct reg1, reg0, reg0, reg0 immoperands1
         %endrep
      %elifidni tmode, L2       ; measure latency from operand 2
         %rep 50 
            instruct reg0, reg1, reg2, reg2 immoperands1
            instruct reg1, reg0, reg3, reg3 immoperands1
         %endrep
      %elifidni tmode, L4       ; measure latency from operand 4
         %rep 50 
            instruct reg0, reg2, reg2, reg1 immoperands1
            instruct reg1, reg3, reg3, reg0 immoperands1
         %endrep
      %elifidni tmode, T         ; measure throughput with register operands
         %rep 50
            instruct reg0, reg1, reg2, reg2 immoperands1
            blockEven
            instruct reg3, reg4, reg5, reg5 immoperands1
            blockOdd
         %endrep
      %elifidni tmode, M         ; measure throughput with memory source operand
         %rep 50
            instruct reg0, reg1, reg2, [rsi] immoperands1
            blockEven
            instruct reg3, reg4, reg5, [rsi+regsize/8] immoperands1
            blockOdd
         %endrep
      %elifidni tmode, M3        ; measure throughput with memory source in operand 3
         %rep 50
            instruct reg0, reg1, [rsi], reg2 immoperands1
            blockEven
            instruct reg3, reg4, [rsi+regsize/8], reg5 immoperands1
            blockOdd
         %endrep
      %else
         %error unknown testmode
      %endif

   %else
      %error unsupported numop
   %endif

   dec loopcounter
   jnz Testloop1   ; loop
%endmacro ; testcode
