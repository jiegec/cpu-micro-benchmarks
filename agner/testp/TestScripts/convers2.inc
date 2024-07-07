; convers2.inc                                                    2013-07-06 AgF
;
; Define test code to test latency and throughput for instructions with
; different register types for input and output. All instructions are in AVX mode.
; (c) Copyright 2012 by Agner Fog. GNU General Public License www.gnu.org/licenses
;
; Parameters:
; tcase:       Test case:
;              xx:  instruct xmm, xmm or any with same size
;              xy:  instruct xmm, ymm
;              yx:  instruct ymm, xmm
;              yyx: instruct ymm, ymm, xmm
;              rx:  instruct r32, xmm
;              ry:  instruct r32, ymm
;
; instruct:    Instruction to test
;
; regsize:     Size of memory operand
;
; numimm:      Number of immediate operands for instruct (0 - 1). Default = 0
;
; immvalue:    Value of first immediate operand for instruct1. Default = 0. 'r' means extra third register operand
;
; tmode:       Test mode:
;              T:   measure throughput of instruct
;
;              L:   measure latency of instruct alone
;
;              LI:  measure latency of instruct + vinserti128 or vextracti128 or movd
;
;              LF:  measure latency of instruct + vinsertf128 or vextractf128
;
;              TM:  measure throughput of instruct with memory operand of size regsize
;
;              TMD: measure throughput of instruct with destination memory operand of size regsize
;
;              TM0: measure throughput of instruct with memory operand of unspecified size
;
;              LM:  measure latency of instruct with memory operand of size regsize
;
;              LMD: measure latency of instruct with destination memory operand of size regsize
;
; -------------------------------------------------------------------------------------------------

%define repeat1  1000        ; loop in TemplateB64.nasm
%define repeat2  1           ; disable loop in TemplateB64.nasm

%ifndef regsize
  %define regsize 128
%endif

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;  xx  ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%ifidni tcase,xx      ;  instruct xmm, xmm

   %ifidni tmode,T        ; measure throughput of instruct

   %macro testcode 0
      %rep 50
      instruct reg0,reg2  immoperands1
      instruct reg1,reg3  immoperands1
      %endrep
   %endmacro

   %elifidni tmode,L      ; measure latency of instruct alone

   %macro testcode 0
      %rep 100
      instruct reg0,reg0  immoperands1
      %endrep
   %endmacro

   %elifidni tmode,TM      ; measure throughput of instruct with memory operand of size regsize

   %macro testcode 0
      %rep 100
      instruct reg0, sizeptr [rsi]  immoperands1
      %endrep
   %endmacro
   
   %elifidni tmode,TM0      ; measure throughput of instruct with memory operand of unspecified size

   %macro testcode 0
      %rep 100
      instruct reg0, [rsi]  immoperands1
      %endrep
   %endmacro
   
   %elifidni tmode,LM      ; measure latency of instruct with memory operand of size regsize

   %macro testcode 0
      %rep 100
      instruct reg0, sizeptr [rsi]  immoperands1
      vmovdqa sizeptr [rsi], reg0
      %endrep
   %endmacro
   
   %else
      %error unknown tmode
   %endif

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;  xy  ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%elifidni tcase,xy      ;  instruct xmm, ymm

   %ifidni tmode,T        ; measure throughput of instruct

   %macro testcode 0
      %rep 50
      instruct xmm0,ymm1  immoperands1
      instruct xmm2,ymm3  immoperands1
      %endrep
   %endmacro

   %elifidni tmode,L      ; measure latency of instruct alone

   %macro testcode 0
      %rep 100
      instruct xmm0,ymm0  immoperands1
      %endrep
   %endmacro

   %elifidni tmode,LI      ; measure latency of instruct + vinserti128

   %macro testcode 0
      %rep 100
      instruct xmm0,ymm1  immoperands1
      vinserti128 ymm1,ymm1,xmm0,1
      %endrep
   %endmacro

   %elifidni tmode,LF      ; measure latency of instruct + vinsertf128

   %macro testcode 0
      %rep 100
      instruct xmm0,ymm1  immoperands1
      vinsertf128 ymm1,ymm1,xmm0,1
      %endrep
   %endmacro

   %elifidni tmode,TM      ; measure throughput of instruct with memory operand of size regsize

   %macro testcode 0
      %rep 100
      instruct xmm0, sizeptr [rsi]  immoperands1
      %endrep
   %endmacro
   
   %elifidni tmode,TMD     ; measure throughput of instruct with memory destination operand of size regsize

   %macro testcode 0
      %rep 100
      instruct sizeptr [rsi], ymm0  immoperands1
      %endrep
   %endmacro
   
   %elifidni tmode,LM      ; measure latency of instruct with memory operand of size regsize

   %macro testcode 0
      %rep 100
      instruct xmm0, sizeptr [rsi]  immoperands1
      %if regsize == 8             ; define appropriate move instruction
        vmovd eax,xmm0
        mov  [rsi], al
      %elif regsize == 16
        vmovd eax,xmm0
        mov  [rsi], ax
      %elif regsize == 32
        vmovd dword [rsi], xmm0
      %elif regsize == 64
        vmovq qword [rsi], xmm0
      %elif regsize == 128
        vmovdqa oword [rsi], xmm0
      %elif regsize == 256
        vmovdqa yword [rsi], ymm0
      %else
        vmovdqa sizeptr [rsi], reg0
      %endif
      %endrep
   %endmacro

   %elifidni tmode,LMD     ; measure latency of instruct with memory destination operand of size regsize
   
   %macro testcode 0
      %rep 100
      instruct sizeptr [rsi], ymm0  immoperands1
      vmovdqa xmm0, sizeptr [rsi]
      %endrep
   %endmacro
   
   %else
      %error unknown tmode
   %endif

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;  yx  ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
   
%elifidni tcase,yx      ;  instruct ymm, xmm

   %ifidni tmode,T        ; measure throughput of instruct

   %macro testcode 0
      %rep 50
      instruct ymm0,xmm1  immoperands1
      instruct ymm2,xmm3  immoperands1
      %endrep
   %endmacro

   %elifidni tmode,L      ; measure latency of instruct alone

   %macro testcode 0
      %rep 100
      instruct ymm0,xmm0  immoperands1
      %endrep
   %endmacro

   %elifidni tmode,LI      ; measure latency of instruct + vextracti128

   %macro testcode 0
      %rep 100
      instruct ymm0,xmm1  immoperands1
      vextracti128 xmm1,ymm0,1
      %endrep
   %endmacro

   %elifidni tmode,LF      ; measure latency of instruct + vextractf128

   %macro testcode 0
      %rep 100
      instruct ymm0,xmm1  immoperands1
      vextractf128 xmm1,ymm0,1
      %endrep
   %endmacro

   %elifidni tmode,TM      ; measure throughput of instruct with memory operand of size regsize

   %macro testcode 0
      %rep 100
      instruct ymm0, sizeptr [rsi]  immoperands1
      %endrep
   %endmacro
   
   %elifidni tmode,TM0      ; measure throughput of instruct with memory operand of unspecified size

   %macro testcode 0
      %rep 100
      instruct ymm0, [rsi]  immoperands1
      %endrep
   %endmacro
   
   %elifidni tmode,LM      ; measure latency of instruct with memory operand of size regsize

   %macro testcode 0
      %rep 100
      instruct ymm0, sizeptr [rsi]  immoperands1
      %if regsize == 8             ; define appropriate move instruction
        vmovd eax,xmm0
        mov  [rsi], al
      %elif regsize == 16
        vmovd eax,xmm0
        mov  [rsi], ax
      %elif regsize == 32
        vmovd dword [rsi], xmm0
      %elif regsize == 64
        vmovq qword [rsi], xmm0
      %elif regsize == 128
        vmovdqa oword [rsi], xmm0
      %elif regsize == 256
        vmovdqa yword [rsi], ymm0
      %else
        vmovdqa sizeptr [rsi], reg0
      %endif
      %endrep
   %endmacro
   
   %else
      %error unknown tmode
   %endif

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;  yyx  ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
   
%elifidni tcase,yyx      ;  instruct ymm, xmm

   %ifidni tmode,T        ; measure throughput of instruct

   %macro testcode 0
      %rep 50
      instruct ymm0,ymm1,xmm2  immoperands1
      instruct ymm3,ymm4,xmm5  immoperands1
      %endrep
   %endmacro

   %elifidni tmode,L      ; measure latency of instruct alone

   %macro testcode 0
      %rep 100
      instruct ymm0,ymm0,xmm0  immoperands1
      %endrep
   %endmacro

   %elifidni tmode,LI      ; measure latency of instruct + vextracti128

   %macro testcode 0
      %rep 100
      instruct ymm0,ymm1,xmm2  immoperands1
      vextracti128 xmm2,ymm0,1
      %endrep
   %endmacro

   %elifidni tmode,LF      ; measure latency of instruct + vextractf128

   %macro testcode 0
      %rep 100
      instruct ymm0,ymm1,xmm2  immoperands1
      vextractf128 xmm2,ymm0,1
      %endrep
   %endmacro

   %elifidni tmode,TM      ; measure throughput of instruct with memory operand of size regsize

   %macro testcode 0
      %rep 100
      instruct ymm0,ymm1,sizeptr [rsi]  immoperands1
      %endrep
   %endmacro
   
   %elifidni tmode,LM      ; measure latency of instruct with memory operand of size regsize

   %macro testcode 0
      %rep 100
      instruct ymm0,ymm1, sizeptr [rsi]  immoperands1
      %if regsize == 8             ; define appropriate move instruction
        vmovd eax,xmm0
        mov  [rsi], al
      %elif regsize == 16
        vmovd eax,xmm0
        mov  [rsi], ax
      %elif regsize == 32
        vmovd dword [rsi], xmm0
      %elif regsize == 64
        vmovq qword [rsi], xmm0
      %elif regsize == 128
        vmovdqa oword [rsi], xmm0
      %elif regsize == 256
        vmovdqa yword [rsi], ymm0
      %else
        vmovdqa sizeptr [rsi], reg0
      %endif
      %endrep
   %endmacro
   
   %else
      %error unknown tmode
   %endif

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;  rx  ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%elifidni tcase,rx      ;  instruct r32, xmm

   %ifidni tmode,T        ; measure throughput of instruct

   %macro testcode 0
      %rep 50
      instruct eax,reg0  immoperands1
      instruct ebx,reg1  immoperands1
      %endrep
   %endmacro

   %elifidni tmode,LI      ; measure latency of instruct + movd

   %macro testcode 0
      %rep 100
      instruct eax,reg1  immoperands1
      vmovd xmm1,eax
      %endrep
   %endmacro

   %elifidni tmode,TM      ; measure throughput of instruct with memory operand of size regsize

   %macro testcode 0
      %rep 100
      instruct eax, sizeptr [rsi]  immoperands1
      %endrep
   %endmacro
   
   %else
      %error unknown tmode
   %endif

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;  ry  ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

%elifidni tcase,ry      ;  instruct r32, ymm

   %ifidni tmode,T        ; measure throughput of instruct

   %macro testcode 0
      %rep 50
      instruct eax,ymm1  immoperands1
      instruct ebx,ymm3  immoperands1
      %endrep
   %endmacro

   %elifidni tmode,LI      ; measure latency of instruct + movd

   %macro testcode 0
      %rep 100
      instruct eax,ymm1  immoperands1
      vmovd xmm1,eax
      %endrep
   %endmacro

   %elifidni tmode,TM      ; measure throughput of instruct with memory operand of size regsize

   %macro testcode 0
      %rep 100
      instruct eax, sizeptr [rsi]  immoperands1
      %endrep
   %endmacro
   
   %else
      %error unknown tmode
   %endif


%else
   %error unknown tcase
%endif
