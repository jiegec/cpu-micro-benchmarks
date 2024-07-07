;----------------------------------------------------------------------------
;                warmup_fp.inc                2015-12-21 Agner Fog
;
; PMC Test program for testing warm up effect of floating point unit
;
; Constants to be defined:
; 
; tcase:   1: integer multiplication
;          2: x87 floating point multiplication
;          3: xmm floating point scalar multiplication
;          4: xmm 128-bit floating point vector multiplication
;          5: ymm 256-bit floating point vector multiplication
; 
; (c) Copyright 2013 - 2015 by Agner Fog. GNU General Public License www.gnu.org/licenses
;-----------------------------------------------------------------------------
; Define any undefined macros

%ifndef tcase
   %define tcase 1
%endif

%define WARMUPCOUNT 0    ; don't use warm up in TemplateB64.nasm

; Let f.p. unit cool down by using integer unit
; (This is actually the same as in TemplateB64.nasm, but that may change:)
%macro testinit1 0
%if tcase == 2   ; use x87
		fld1
		fld1
%endif

%ifdef primingdelay
        vxorps ymm0,ymm0,ymm0
        mov ecx, primingdelay/20
        mov eax, 1
        align 16
Wuloop1:
        %rep 20
        add eax,eax
        %endrep
        dec ecx
        jnz Wuloop1
%endif
%endmacro

; define counts in warmup_fp.sh2
; %define repeat0 20
; %define repeat1 10
; %define repeat2 10


; Define test cases

%if tcase == 1   ; integer multiplication

   %macro testcode 0
      imul rax, rbx
   %endmacro

%elif tcase == 2   ; x87 floating point multiplication

   %macro testcode 0
      ;fmul st(1),st(0)
	  fmul st1,st0
   %endmacro

%elif tcase == 3   ; xmm floating point scalar multiplication

   %macro testcode 0
      mulsd xmm1,xmm2
   %endmacro

%elif tcase == 4   ; xmm 128-bit floating point vector multiplication

   %macro testcode 0
      mulpd xmm1,xmm2
   %endmacro

%elif tcase == 5   ; ymm 256-bit floating point vector multiplication latency

   %macro testcode 0
      vmulpd ymm1,ymm1,ymm2
   %endmacro

%elif tcase == 6   ; ymm 256-bit floating point vector multiplication throughput

   %macro testcode 0
      vmulpd ymm1,ymm2,ymm2
      vmulpd ymm3,ymm4,ymm4
   %endmacro

%else
   %error unknown test case tcase
%endif
