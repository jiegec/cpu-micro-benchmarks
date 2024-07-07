; convers.inc                                                              2012-02-01 AgF
;
; Define universal test code to test latency and throughput for conversion instructions with
; different register types for input and output
; (c) Copyright 2012 by Agner Fog. GNU General Public License www.gnu.org/licenses
;
; Parameters:
;
; instruct1:   Instruction to convert from A to B
;
; instruct2:   Instruction to convert from B to A
;
; instruct3:   Possible extra chain instruction with A type registers for both input and output
;
; regtype1:    Register type for operand type A (r8, r16, r32, r64, mmx, xmm, ymm, m, m8, m16, m32, m64, m128, m256)
;
; regtype2:    Register type for operand type B (r8, r16, r32, r64, mmx, xmm, ymm, m, m8, m16, m32, m64, m128, m256)
;
; numimm:      Number of immediate operands for instruct1 (0 - 1). Default = 0
;
; numimm2:     Number of immediate operands for instruct2 (0 - 1). Default = 0
;
; immvalue:    Value of first immediate operand for instruct1. Default = 0. 'r' means extra third register operand
;
; immvalue2:   Value of first immediate operand for instruct2. Default = 0.
;
; numop1:      Number of register (or memory) operands for instruct1. Default = 2, max = 3
;
; tmode:       Test mode:
;              T1:   measure throughput of instruct1 B, A
;
;              M1:   measure latency of instruct1 B, [m]
;
;              L1:   measure latency of instruct1 A, A if regtype1 and regtype2 are the same
;
;              L12:  measure latency of instruct1 B, A + instruct2 A, B
;
;              L123: measure latency of instruct1 B, A + instruct2 A, B + instruct3 A, A
;
;              LSX:  measure latency of movsx or movzx with memory operand
;
; -------------------------------------------------------------------------------------------------

%ifndef repeat2
%define repeat2 100
%endif

; select register names of desired types:

%ifidni regtype1, r8
   %define RA1 al
   %define RA2 bl
   %define RA3 cl
   %define RA4 dl
%elifidni regtype1, r8h
   %define RA1 ah
   %define RA2 bh
   %define RA3 ch
   %define RA4 dh
%elifidni regtype1, r16
   %define RA1 ax
   %define RA2 bx
   %define RA3 cx
   %define RA4 dx
%elifidni regtype1, r32
   %define RA1 eax
   %define RA2 ebx
   %define RA3 ecx
   %define RA4 edx
%elifidni regtype1, r64
   %define RA1 rax
   %define RA2 rbx
   %define RA3 rcx
   %define RA4 rdx
%elifidni regtype1, mmx
   %define RA1 mm0
   %define RA2 mm1
   %define RA3 mm2
   %define RA4 mm3
%elifidni regtype1, xmm
   %define RA1 xmm0
   %define RA2 xmm1
   %define RA3 xmm2
   %define RA4 xmm3
%elifidni regtype1, ymm
   %define RA1 ymm0
   %define RA2 ymm1
   %define RA3 ymm2
   %define RA4 ymm3
%elifidni regtype1, m
   %define RA1 [psi]
   %define RA2 [rdi]
   %define RA3 [rsi]
   %define RA4 [rdi]
%elifidni regtype1, m8
   %define RA1 byte [psi]
   %define RA2 byte [rdi]
   %define RA3 byte [rsi]
   %define RA4 byte [rdi]
%elifidni regtype1, m16
   %define RA1 word [psi]
   %define RA2 word [rdi]
   %define RA3 word [rsi]
   %define RA4 word [rdi]
%elifidni regtype1, m32
   %define RA1 dword [psi]
   %define RA2 dword [rdi]
   %define RA3 dword [rsi]
   %define RA4 dword [rdi]
%elifidni regtype1, m64
   %define RA1 qword [psi]
   %define RA2 qword [rdi]
   %define RA3 qword [rsi]
   %define RA4 qword [rdi]
%elifidni regtype1, m128
   %define RA1 oword [psi]
   %define RA2 oword [rdi]
   %define RA3 oword [rsi]
   %define RA4 oword [rdi]
%elifidni regtype1, m256
   %define RA1 yword [psi]
   %define RA2 yword [rdi]
   %define RA3 yword [rsi]
   %define RA4 yword [rdi]
%else
   %error unknown regtype1
%endif

%ifndef regtype2
   %define regtype2 regtype1
%endif

%ifidni regtype2, r8
   %define RB1 al
   %define RB2 bl
   %define RB3 cl
   %define RB4 dl
%elifidni regtype2, r8h
   %define RB1 ah
   %define RB2 bh
   %define RB3 ch
   %define RB4 dh
%elifidni regtype2, r16
   %define RB1 ax
   %define RB2 bx
   %define RB3 cx
   %define RB4 dx
%elifidni regtype2, r32
   %define RB1 eax
   %define RB2 ebx
   %define RB3 ecx
   %define RB4 edx
%elifidni regtype2, r64
   %define RB1 rax
   %define RB2 rbx
   %define RB3 rcx
   %define RB4 rdx
%elifidni regtype2, mmx
   %define RB1 mm0
   %define RB2 mm1
   %define RB3 mm2
   %define RB4 mm3
%elifidni regtype2, xmm
   %define RB1 xmm0
   %define RB2 xmm1
   %define RB3 xmm2
   %define RB4 xmm3
%elifidni regtype2, ymm
   %define RB1 ymm0
   %define RB2 ymm1
   %define RB3 ymm2
   %define RB4 ymm3
%elifidni regtype2, m
   %define RB1 [psi]
   %define RB2 [rdi]
   %define RB3 [rsi]
   %define RB4 [rdi]
%elifidni regtype2, m8
   %define RB1 byte [psi]
   %define RB2 byte [rdi]
   %define RB3 byte [rsi]
   %define RB4 byte [rdi]
%elifidni regtype2, m16
   %define RB1 word [psi]
   %define RB2 word [rdi]
   %define RB3 word [rsi]
   %define RB4 word [rdi]
%elifidni regtype2, m32
   %define RB1 dword [psi]
   %define RB2 dword [rdi]
   %define RB3 dword [rsi]
   %define RB4 dword [rdi]
%elifidni regtype2, m64
   %define RB1 qword [psi]
   %define RB2 qword [rdi]
   %define RB3 qword [rsi]
   %define RB4 qword [rdi]
%elifidni regtype2, m128
   %define RB1 oword [psi]
   %define RB2 oword [rdi]
   %define RB3 oword [rsi]
   %define RB4 oword [rdi]
%elifidni regtype2, m256
   %define RB1 yword [psi]
   %define RB2 yword [rdi]
   %define RB3 yword [rsi]
   %define RB4 yword [rdi]
%else
   %error unknown regtype1
%endif

; define immediate operands for instruction 2 (immoperands1 is in templateB64.nasm)
%ifndef numimm2
   %define numimm2  0  ; default number of immediate operands for instruction 2
%endif
%ifndef immvalue2
   %define immvalue2  0  ; default value of immediate operands for instruction 2
%endif
%if numimm2 == 0
   %define immoperands2
%elif numimm2 == 1
   %define immoperands2 , immvalue2
%endif

; initialization of registers
%macro testinit2 0
   xor eax, eax
%endmacro

%ifndef numop1
   %define numop1 2
%endif


; test code for each mode:

%if numop1 == 2

%ifidni tmode,T1        ; measure throughput of instruct1 B, A   

   %macro testcode 0
      instruct1 RB1, RA2 immoperands1
      instruct1 RB4, RA3 immoperands1
   %endmacro

%elifidni tmode,M1      ; measure throughput of instruct1 B, [mem]   

   %macro testcode 0
      instruct1 RB1, RA1 immoperands1
      instruct1 RB2, RA1 immoperands1
   %endmacro

%elifidni tmode,L1        ; measure latency of instruct1 A, A

   %macro testcode 0
      instruct1 RB1, RA2 immoperands1
      instruct1 RB2, RA1 immoperands1
   %endmacro

%elifidni tmode,L12        ; measure latency of instruct1 B, A + instruct2 A, B

   %macro testcode 0
      instruct1 RB2, RA1 immoperands1
      instruct2 RA1, RB2 immoperands2
   %endmacro

%elifidni tmode,L123        ; measure latency of instruct1 B, A + instruct2 A, B + instruct3 A, A

   %macro testcode 0
      instruct1 RB2, RA1 immoperands1
      instruct2 RA1, RB2 immoperands2
      instruct3 RA1, RA1
   %endmacro

%elifidni tmode,LSX         ; measure latency of movsx r,[m] + mov [m],r

   %macro testcode 0
      instruct1 RB1, RA1
      %ifidni regtype1,m8
         mov byte [rsi], al
      %elifidni regtype1,m16
         mov word [rsi], ax
      %elifidni regtype1,m32
         mov dword [rsi], eax
      %else
         %error unknown combination of operands
      %endif
   %endmacro

%else
   %error unknown tmode
%endif

%elif numop1 == 3

%ifidni tmode,T1        ; measure throughput of instruct1 B, A   

   %macro testcode 0
      instruct1 RB1, RA2, RA2 immoperands1
      instruct1 RB4, RA3, RA3 immoperands1
   %endmacro

%elifidni tmode,M1      ; measure throughput of instruct1 B, [mem]

   %macro testcode 0
      instruct1 RB1, RA2, [psi] immoperands1
      instruct1 RB3, RA4, [psi] immoperands1
   %endmacro

%elifidni tmode,L1        ; measure latency of instruct1 A, A

   %macro testcode 0
      instruct1 RB1, RA2, RA2 immoperands1
      instruct1 RB2, RA1, RA1 immoperands1
   %endmacro

%elifidni tmode,L12        ; measure latency of instruct1 B, A + instruct2 A, B

   %macro testcode 0
      instruct1 RB2, RA1, RA1 immoperands1
      instruct2 RA1, RB2      immoperands2
   %endmacro

%elifidni tmode,L123        ; measure latency of instruct1 B, A + instruct2 A, B + instruct3 A, A

   %macro testcode 0
      instruct1 RB2, RA1, RA1 immoperands1
      instruct2 RA1, RB2      immoperands2
      instruct3 RA1, RA1
   %endmacro

%endif

%else
   %error unknown numop1
%endif
