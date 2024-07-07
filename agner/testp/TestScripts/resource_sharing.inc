;----------------------------------------------------------------------------
;        resource_sharing.inc                          2013-07-09 Agner Fog
;
; PMC Test program for testing whether various resources are shared between
; multiple CPU cores
;
; Constants to be defined:
; 
; tcase:   1: Instruction fetch. Execute 12-byte long NOPs
;          2: Instruction decode. Execute short NOPs
;          3: Integer instructions, Add
;          4: Integer instructions, Multiply
;          5: Integer instructions, Divide
;          6: Conditional jumps
;          7: Floating point x87 instructions, Add
;          8: Integer vector instructions, Add + multiply. Define regsize = data size. 128 = SSE2, 256 = AVX2
;          9: Floating point vector instructions, Add + multiply. Define regsize = data size. 128 = SSE2, 256 = AVX
;         10: Floating point vector instructions, Divide. Define regsize = data size. 128 = SSE2, 256 = AVX
;         11: Memory read. Define regsize = data size. 256 requires AVX
;         12: Memory write. Define regsize = data size. 256 requires AVX
;         13: Memory read + write. Define regsize = data size. 256 requires AVX
; 
; nthreads:     Number of simultaneous threads (default = 3)
; 
; counters:     A comma-separated list of PMC counter numbers 
;               (referring to CounterDefinitions in PMCTestA.cpp)
;
; (c) Copyright 2013 by Agner Fog. GNU General Public License www.gnu.org/licenses
;-----------------------------------------------------------------------------
; Define any undefined macros

; define long nops
%ifndef noptype
   %define noptype 2
%endif

%include "nops.inc"


%ifndef tcase
   %define tcase 1
%endif

; Define test cases

%if tcase == 1   ; 1: Instruction fetch. Execute long NOPs

   %macro testcode 0
      %rep 100
         nop12
      %endrep      
   %endmacro

%elif tcase == 2   ; 2: Instruction decode. Execute short NOPs

   %macro testcode 0
      %rep 100
         nop
      %endrep      
   %endmacro

%elif tcase == 3   ; 3: Integer instructions, Add

   %macro testcode 0
      %rep 10
         add eax,eax
         add ebx,ebx
         cmp esi,1
         cmp edi,1
         cmp ebp,1
         add ecx,ecx
         add edx,edx
         cmp esi,1
         cmp edi,1
         cmp ebp,1
      %endrep      
   %endmacro

%elif tcase == 4   ; 4: Integer instructions, Multiply

   %macro testcode 0
      %rep 25
         imul eax,eax
         imul ebx,ebx
         imul ecx,ecx
         imul edx,edx
      %endrep      
   %endmacro

%elif tcase == 5   ; 5: Integer instructions, Divide

   %macro testinit2 0
      mov eax,0x407bde53
      mov edx,0x25d3
      mov ebx,0x9a07
      mov ecx,0x1f42
   %endmacro

   %macro testcode 0
      %rep 100
         idiv ebx
         mov edx, ecx
      %endrep      
   %endmacro

%elif tcase == 6   ; 6: Conditional jumps

   %macro testinit2 0
      mov rbx,0x9a075d23e1b64f8c
   %endmacro

   %macro testcode 0
      %rep 100
         ror rbx,1
         jc $+3
         nop
      %endrep      
   %endmacro

%elif tcase == 7   ; 7: Floating point x87 instructions, Add

   %macro testinit2 0
      fldz
      fldz
      fldz
      fldz
      fldz
      fldz
   %endmacro

   %macro testcode 0
      %rep 20
         fadd st1,st0
         fadd st2,st0
         fadd st3,st0
         fadd st4,st0
         fadd st5,st0
      %endrep      
   %endmacro
   
   %macro testafter2 0
      finit
   %endmacro

%elif tcase == 8   ; 8: Integer vector instructions, Add + multiply

   %macro testcode 0
      %if regsize < 256    
         %rep 20
            paddd xmm0,xmm0
            paddd xmm1,xmm1
            pmullw xmm2,xmm2
            pmullw xmm3,xmm3
            movdqa xmm4,xmm4         
         %endrep      
      %else  ; 256
         %rep 20
            vpaddd ymm0,ymm0
            vpaddd ymm1,ymm1
            vpmullw ymm2,ymm2
            vpmullw ymm3,ymm3
            vmovdqa ymm4,ymm4         
         %endrep      
      %endif
   %endmacro

%elif tcase == 9   ; 9: Floating point vector instructions, Add + multiply

   %macro testcode 0
      %if regsize < 256    
         %rep 20
            addps xmm0,xmm0
            addps xmm1,xmm1
            mulps xmm2,xmm2
            mulps xmm3,xmm3
            movaps xmm4,xmm4         
         %endrep      
      %else  ; 256
         %rep 20
            vaddps ymm0,ymm0
            vaddps ymm1,ymm1
            vmulps ymm2,ymm2
            vmulps ymm3,ymm3
            vmovaps ymm4,ymm4         
         %endrep      
      %endif
   %endmacro

%elif tcase == 10  ; 10: Floating point vector instructions, Divide

   %macro testdata 0
      dat1 dq 3.729266052045E100, 3.729266052045E100, 3.729266052045E100, 3.729266052045E100
      dat2 dq 1.356051603609, 1.356051603609, 1.356051603609, 1.356051603609
   %endmacro
   
   %macro testinit3 0
      %if regsize < 256
         movapd xmm1,[dat1]
         movapd xmm2,[dat2]
      %else
         vmovapd ymm1,[dat1]
         vmovapd ymm2,[dat2]
      %endif   
   %endmacro
   
   %macro testcode 0
      %if regsize < 256    
         %rep 100
            divpd xmm1,xmm2
         %endrep      
      %else
         %rep 100
            vdivpd ymm1,ymm2
         %endrep      
      %endif   
   %endmacro

%elif tcase == 11  ; 11: Memory read. Define regsize = data size

   %macro testcode 0
      %if regsize < 128
         %rep 20
            mov reg0, [psi]
            mov reg1, [psi+8]
            mov reg2, [psi+0x10]
            mov reg3, [psi+0x18]
            mov reg4, [psi+0x20]
         %endrep      
      %elif regsize == 128
         %rep 20
            movaps reg0, [psi]
            movaps reg1, [psi+0x10]
            movaps reg2, [psi+0x20]
            movaps reg3, [psi+0x30]
            movaps reg4, [psi+0x40]
         %endrep      
      %elif regsize == 256
         %rep 20
            vmovaps reg0, [psi]
            vmovaps reg1, [psi+0x20]
            vmovaps reg2, [psi+0x40]
            vmovaps reg3, [psi+0x60]
            vmovaps reg4, [psi+0x80]
         %endrep
      %else
         %error unknown register size regsize
      %endif   
   %endmacro

%elif tcase == 12  ; 12: Memory write. Define regsize = data size

   %macro testcode 0
      %if regsize < 128
         %rep 20
            mov [psi], reg0
            mov [psi+8], reg1
            mov [psi+0x10], reg2
            mov [psi+0x18], reg3
            mov [psi+0x20], reg4
         %endrep      
      %elif regsize == 128
         %rep 20
            movaps [psi], reg0
            movaps [psi+0x10], reg1
            movaps [psi+0x20], reg2
            movaps [psi+0x30], reg3
            movaps [psi+0x40], reg4
         %endrep      
      %elif regsize == 256
         %rep 20
            vmovaps [psi], reg0
            vmovaps [psi+0x20], reg1
            vmovaps [psi+0x40], reg2
            vmovaps [psi+0x60], reg3
            vmovaps [psi+0x80], reg4
         %endrep
      %else
         %error unknown register size regsize
      %endif   
   %endmacro

%elif tcase == 13  ; 13: Memory read + write. Define regsize = data size

   %macro testcode 0
      %if regsize < 128
         %rep 20
            mov reg0, [psi]
            mov reg1, [psi+8]
            mov [psi+0x10], reg2
            mov reg3, [psi+0x18]
            mov [psi+0x20], reg4
         %endrep      
      %elif regsize == 128
         %rep 20
            movaps reg0, [psi]
            movaps reg1, [psi+0x10]
            movaps [psi+0x20], reg2
            movaps reg3, [psi+0x30]
            movaps [psi+0x40], reg4
         %endrep      
      %elif regsize == 256
         %rep 20
            vmovaps reg0, [psi]
            vmovaps reg1, [psi+0x20]
            vmovaps [psi+0x40], reg2
            vmovaps reg3, [psi+0x60]
            vmovaps [psi+0x80], reg4
         %endrep
      %else
         %error unknown register size regsize
      %endif   
   %endmacro

%else
   %error unknown test case tcase
%endif

; default test loops
%define repeat1 1000
%define repeat2 1
