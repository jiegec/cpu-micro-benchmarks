; store_forwarding.inc                 2021-03-23 Agner Fog

; Test cache access time
; (c) 2013-2021 by Agner Fog. GNU General Public License www.gnu.org/licenses

; Parameters:
;
; regsize:   size of write operand, default = 32
; readsize:  size of read after write, default = 32
; roffset:   offset of read after write, default = 0
; multipart: read in multiple parts, default = 1, valid values are 1, 2, 4, 8
; crossbound: write crosses memory address divisible by this value (0 = aligned), default = 0, max = 256
; addressmode: addressing mode: indirect(default), abs32, abs64, rip

%ifndef roffset
   %define roffset 0
%endif

%ifndef multipart
   %define multipart 1
%endif

%ifndef crossbound
   %define crossbound 0
%endif

%ifndef addressmode
   %define addressmode indirect
%endif

%ifndef readsize
   %define readsize 32
%endif

%if readsize == 8             ; define registers of desired size
   %define readreg0  al
   %define readreg1  bl
   %define readreg2  cl
   %define readreg3  dl
   %define mov2 mov
   %define sizeptr2 byte   
%elif readsize == 16
   %define readreg0  ax
   %define readreg1  bx
   %define readreg2  cx
   %define readreg3  dx
   %define mov2 mov
   %define sizeptr2 word   
%elif readsize == 32
   %if regsize < 128
      %define readreg0  eax
      %define readreg1  ebx
      %define readreg2  ecx
      %define readreg3  edx
      %define mov2 mov
      %define sizeptr2 dword
   %elif regsize == 128
      %define readreg0  xmm0
      %define readreg1  xmm1
      %define readreg2  xmm2
      %define readreg3  xmm3
      %define mov2 movd
      %define sizeptr2 dword   
   %else
      %define readreg0  xmm0
      %define readreg1  xmm1
      %define readreg2  xmm2
      %define readreg3  xmm3
      %define mov2 vmovd
      %define sizeptr2 dword   
   %endif
     
%elif readsize == 64
   %if regsize < 128
      %define readreg0  rax
      %define readreg1  rbx
      %define readreg2  rcx
      %define readreg3  rdx
      %define mov2 mov
      %define sizeptr2 qword
   %elif regsize == 128
      %define readreg0  xmm0
      %define readreg1  xmm1
      %define readreg2  xmm2
      %define readreg3  xmm3
      %define mov2 movq
      %define sizeptr2 qword   
   %else
      %define readreg0  xmm0
      %define readreg1  xmm1
      %define readreg2  xmm2
      %define readreg3  xmm3
      %define mov2 vmovq
      %define sizeptr2 qword   
   %endif
%elif readsize == 65    ; 64 bit mmx registers
   %define readreg0  mm0
   %define readreg1  mm1
   %define readreg2  mm2
   %define readreg3  mm3
   %define mov2 movq
   %define sizeptr2 mmword   
%elif readsize == 128
   %define readreg0  xmm0
   %define readreg1  xmm1
   %define readreg2  xmm2
   %define readreg3  xmm3
   %if regsize > 128
      %define mov2 vmovdqu
   %else
      %define mov2 movdqu
   %endif
   %define sizeptr2 oword   
%elif readsize == 256
   %define readreg0  ymm0
   %define readreg1  ymm1
   %define readreg2  ymm2
   %define readreg3  ymm3
   %define mov2 vmovdqu
   %define sizeptr2 yword   
%elif readsize == 512
   %define readreg0  zmm0
   %define readreg1  zmm1
   %define readreg2  zmm2
   %define readreg3  zmm3
   %define mov2 vmovdqu32
   %define sizeptr2 zword   
%else
   %error unknown readsize
%endif

%if regsize <= 64
   %define mov1 mov
%elif regsize == 65
   %define mov1 movq
%elif regsize == 128
   %define mov1 movdqu
%elif regsize == 256
   %define mov1 vmovdqu
%elif regsize == 512
   %define mov1 vmovdqu32
%else
   %define mov1 vmovdqu
%endif



; initialization of aligned or misaligned pointer
%macro testinit1 0
    lea psi, [UserData + 1000h]
    and psi, -100h
    %if crossbound > 1
      sub psi, crossbound + (regsize/8+1)/2
    %endif
%endmacro


; main testcode macro
%ifidni addressmode, indirect

%if multipart == 1
    %macro testcode 0
    mov1 sizeptr [rsi], reg0
    mov2 readreg0, sizeptr2 [rsi+roffset]
    %endmacro
%elif multipart == 2 && readsize <= regsize
    %macro testcode 0
    mov1 sizeptr [rsi], reg0
    mov2 readreg1, sizeptr2 [rsi+roffset]
    mov2 readreg0, sizeptr2 [rsi+roffset+readsize/8]
    %endmacro
%elif multipart == 4 && readsize <= regsize
    %macro testcode 0
    mov1 sizeptr [rsi], reg0
    mov2 readreg3, sizeptr2 [rsi+roffset]
    mov2 readreg2, sizeptr2 [rsi+roffset+readsize/8]
    mov2 readreg1, sizeptr2 [rsi+roffset+2*readsize/8]
    mov2 readreg0, sizeptr2 [rsi+roffset+3*readsize/8]
    %endmacro
%elif multipart == 8 && readsize <= regsize
    %macro testcode 0
    %if regsize == 64
       mov [rsi],rax
       mov bl,[rsi]
       mov cl,[rsi+1]
       mov bl,[rsi+2]
       mov cl,[rsi+3]
       mov bl,[rsi+4]
       mov cl,[rsi+5]
       mov bl,[rsi+6]
       movzx eax, byte [rsi+7]
    %elif regsize == 128
       movdqa [rsi], xmm0
       movzx ebx, word [rsi]
       movzx ecx, word [rsi+2]
       movzx ebx, word [rsi+4]
       movzx ecx, word [rsi+6]
       movzx ebx, word [rsi+8]
       movzx ecx, word [rsi+10]
       movzx ebx, word [rsi+12]
       movzx eax, word [rsi+14]
       movd xmm0, eax
    %elif regsize == 256
       vmovdqa [rsi], ymm0
       vmovd xmm1, [rsi]
       vmovd xmm2, [rsi+4]
       vmovd xmm1, [rsi+8]
       vmovd xmm2, [rsi+0Ch]
       vmovd xmm1, [rsi+10h]
       vmovd xmm2, [rsi+14h]
       vmovd xmm1, [rsi+18h]
       vmovd xmm0, [rsi+1ch]    
    %elif regsize == 512
       vmovdqa64 [rsi], zmm0
       vmovq xmm1, [rsi]
       vmovq xmm2, [rsi+8]
       vmovq xmm3, [rsi+10h]
       vmovq xmm4, [rsi+18h]
       vmovq xmm5, [rsi+20h]
       vmovq xmm6, [rsi+28h]
       vmovq xmm7, [rsi+30h]
       vmovq xmm8, [rsi+38h]    
    %else
       %error register size regsize cannot be divided into eight
    %endif
    %endmacro
    
%elif multipart == 2 && readsize > regsize

    %macro testcode 0
    %if readsize == 64
       mov [rsi], eax
       mov [rsi+4], eax
       mov rax, [rsi]
    %elif readsize == 128
       movq [rsi], xmm0
       movq [rsi+8], xmm0
       movdqa xmm0, [rsi]
    %elif readsize == 256
       vmovdqa [rsi], xmm0
       vmovdqa [rsi+10h], xmm0
       vmovdqa ymm0, [rsi]
    %else
       %error wrong register size for multipart write
    %endif
    %endmacro

%else
   %error unknown multipart value
%endif

%elifidni addressmode, rip

    %macro testcode 0
       mov1 [rel UserData], readreg0
       mov1 readreg0, [rel UserData]
    %endmacro

%elifidni addressmode, abs32

    %macro testcode 0
       mov1 [UserData], readreg0
       mov1 readreg0, [UserData]
    %endmacro

%elifidni addressmode, abs64

    %macro testcode 0
       mov [abs qword UserData], readreg0
       mov readreg0, [abs qword UserData]
    %endmacro

%else

   %error unknown addressing mode addressmode
   
%endif


; test loops
%define repeat1 1000
%define repeat2 100
