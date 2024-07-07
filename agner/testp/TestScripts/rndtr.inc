; rndtr.inc                                                           2016-10-12 Agner Fog
;
; Define test code to test latency of a round-trip sequence of instructions with
; different register types. Can also test throughput
; (c) Copyright 2016 by Agner Fog. GNU General Public License www.gnu.org/licenses
;
; Parameters:
;
; instruct1, instruct2, instruct3: Up to three instructions in round trip
; instruct2 or instruct3 (but not both) can be "n" to indicate an implicit convertion between
; different sizes of the same register.
;
; regtype1, regtype2, regtype3. Register or operand types (see runlist.sh for defined values):
; regtype1 is source for first instruction and destination for the last instruction
; regtype2 is source for second instruction and destination for the first
; regtype3 is source for third instruction and destination for the second
;
; regsize1, regsize2, regsize3: Register or operand sizes for these operands
;
; numop1, numop2, numop3: Number of register or memory operands for each instruction. Default = 2, max = 3
;   A value of "3x" indicates source operands of mixed type. The first source operand has the same type as the destination operand.
;
; numimm1, numimm2, numimm3: Number of immediate operands for each instruction. Default = 0
;
; immvalue1, immvalue2, immvalue3:  Value of immediate operands for each instruction. Default = 0
;
; tmode:       Test mode:
;              L:   measure round trip of instructions
;              T1, T2, T3:  Measure throughput of one of the instructions
;
; -------------------------------------------------------------------------------------------------

; define undefined parameters

%ifndef tmode
   %define tmode L
%endif

%ifndef repeat2
   %define repeat2 100
%endif

%ifndef instruct2
   %define instruct2
%endif

%ifndef instruct3
   %define instruct3
%endif

%ifndef numimm2
   %define numimm2 0
%endif

%ifndef numimm3
   %define numimm3 0
%endif

; count number of instructions
%ifnempty instruct3
   %define numinstr 3
%elifnempty instruct2
   %define numinstr 2
%elifnempty instruct1
   %define numinstr 1
   %error must have at least two instructions
%else
   %error no instructions defined
%endif

; check if any instructions have mixed operand types
%assign instmix1 0
%assign instmix2 0
%assign instmix3 0
%ifidni numop1, 3x
   %undef numop1
   %define numop1 3
   %assign instmix1 1
%endif
%ifidni numop2, 3x
   %undef numop2
   %define numop2 3
   %assign instmix2 1
%endif
%ifidni numop3, 3x
   %undef numop3
   %define numop3 3
   %assign instmix3 1
%endif
%ifidni regtype1, m
   %assign instmix1 1
%endif
%ifidni regtype2, m
   %assign instmix2 1
%endif
%ifidni regtype3, m
   %assign instmix3 1
%endif


; select register names of desired types:

%ifidni regtype1, r
   %if regsize1 == 8
      %define ra1 al
      %define ra2 bl
      %define ra3 cl
      %define ra4 dl
   %elif regsize1 == 16
      %define ra1 ax
      %define ra2 bx
      %define ra3 cx
      %define ra4 dx
   %elif regsize1 == 32
      %define ra1 eax
      %define ra2 ebx
      %define ra3 ecx
      %define ra4 edx
   %elif regsize1 == 64
      %define ra1 rax
      %define ra2 rbx
      %define ra3 rcx
      %define ra4 rdx
   %else
      %error unknown register size 1
   %endif
%elifidni regtype1, h
   %define ra1 ah
   %define ra2 bh
   %define ra3 ch
   %define ra4 dh
%elifidni regtype1, mmx
   %define ra1 mm0
   %define ra2 mm1
   %define ra3 mm2
   %define ra4 mm3
%elifidni regtype1, k
   %define ra1 k1
   %define ra2 k2
   %define ra3 k3
   %define ra4 k4
%elifidni regtype1, v
   %if regsize1 == 128
      %define ra1 xmm0
      %define ra2 xmm1
      %define ra3 xmm2
      %define ra4 xmm3
   %elif regsize1 == 256
      %define ra1 ymm0
      %define ra2 ymm1
      %define ra3 ymm2
      %define ra4 ymm3
   %elif regsize1 == 512
      %define ra1 zmm0
      %define ra2 zmm1
      %define ra3 zmm2
      %define ra4 zmm3
   %else
      %error unknown register size 1
   %endif
%elifidni regtype1, m
   %if regsize1 == 0
      %define sizeptr1
   %elif regsize1 == 8
      %define sizeptr1 byte
   %elif regsize1 == 16
      %define sizeptr1 word
   %elif regsize1 == 32
      %define sizeptr1 dword
   %elif regsize1 == 64
      %define sizeptr1 qword
   %elif regsize1 == 128
      %define sizeptr1 oword
   %elif regsize1 == 256
      %define sizeptr1 yword
   %elif regsize1 == 512
      %define sizeptr1 zword
   %else
      %error unknown register size 1
   %endif
   %define ra1 sizeptr1 [psi]
   %define ra2 sizeptr1 [rdi]
   %define ra3 sizeptr1 [rsi]
   %define ra4 sizeptr1 [rdi]
%else
   %error unknown register type 1 regtype1
%endif

%if numinstr > 1
%ifidni regtype2, r
   %if regsize2 == 8
      %define rb1 al
      %define rb2 bl
      %define rb3 cl
      %define rb4 dl
   %elif regsize2 == 16
      %define rb1 ax
      %define rb2 bx
      %define rb3 cx
      %define rb4 dx
   %elif regsize2 == 32
      %define rb1 eax
      %define rb2 ebx
      %define rb3 ecx
      %define rb4 edx
   %elif regsize2 == 64
      %define rb1 rbx
      %define rb2 rbx
      %define rb3 rcx
      %define rb4 rdx
   %else
      %error unknown register size 2
   %endif
%elifidni regtype2, h
   %define rb1 ah
   %define rb2 bh
   %define rb3 ch
   %define rb4 dh
%elifidni regtype2, mmx
   %define rb1 mm0
   %define rb2 mm1
   %define rb3 mm2
   %define rb4 mm3
%elifidni regtype2, k
   %define rb1 k1
   %define rb2 k2
   %define rb3 k3
   %define rb4 k4
%elifidni regtype2, v
   %if regsize2 == 128
      %define rb1 xmm0
      %define rb2 xmm1
      %define rb3 xmm2
      %define rb4 xmm3
   %elif regsize2 == 256
      %define rb1 ymm0
      %define rb2 ymm1
      %define rb3 ymm2
      %define rb4 ymm3
   %elif regsize2 == 512
      %define rb1 zmm0
      %define rb2 zmm1
      %define rb3 zmm2
      %define rb4 zmm3
   %else
      %error unknown register size 2
   %endif
%elifidni regtype2, m
   %if regsize2 == 0
      %define sizeptr2
   %elif regsize2 == 8
      %define sizeptr2 byte
   %elif regsize2 == 16
      %define sizeptr2 word
   %elif regsize2 == 32
      %define sizeptr2 dword
   %elif regsize2 == 64
      %define sizeptr2 qword
   %elif regsize2 == 128
      %define sizeptr2 oword
   %elif regsize2 == 256
      %define sizeptr2 yword
   %elif regsize2 == 512
      %define sizeptr2 zword
   %else
      %error unknown register size 2
   %endif
   %define rb1 sizeptr2 [psi]
   %define rb2 sizeptr2 [rdi]
   %define rb3 sizeptr2 [rsi]
   %define rb4 sizeptr2 [rdi]
%else
   %error unknown register type 2 regtype2
%endif
%endif

%if numinstr > 2
%ifidni regtype3, r
   %if regsize3 == 8
      %define rc1 al
      %define rc2 bl
      %define rc3 cl
      %define rc4 dl
   %elif regsize3 == 16
      %define rc1 ax
      %define rc2 bx
      %define rc3 cx
      %define rc4 dx
   %elif regsize3 == 32
      %define rc1 eax
      %define rc2 ebx
      %define rc3 ecx
      %define rc4 edx
   %elif regsize3 == 64
      %define rc1 rcx
      %define rc2 rcx
      %define rc3 rcx
      %define rc4 rdx
   %else
      %error unknown register size 2
   %endif
%elifidni regtype3, h
   %define rc1 ah
   %define rc2 bh
   %define rc3 ch
   %define rc4 dh
%elifidni regtype3, mmx
   %define rc1 mm0
   %define rc2 mm1
   %define rc3 mm2
   %define rc4 mm3
%elifidni regtype3, k
   %define rc1 k1
   %define rc2 k2
   %define rc3 k3
   %define rc4 k4
%elifidni regtype3, v
   %if regsize3 == 128
      %define rc1 xmm0
      %define rc2 xmm1
      %define rc3 xmm2
      %define rc4 xmm3
   %elif regsize3 == 256
      %define rc1 ymm0
      %define rc2 ymm1
      %define rc3 ymm2
      %define rc4 ymm3
   %elif regsize3 == 512
      %define rc1 zmm0
      %define rc2 zmm1
      %define rc3 zmm2
      %define rc4 zmm3
   %else
      %error unknown register size 2
   %endif
%elifidni regtype3, m
   %if regsize3 == 0
      %define sizeptr3
   %elif regsize3 == 8
      %define sizeptr3 byte
   %elif regsize3 == 16
      %define sizeptr3 word
   %elif regsize3 == 32
      %define sizeptr3 dword
   %elif regsize3 == 64
      %define sizeptr3 qword
   %elif regsize3 == 128
      %define sizeptr3 oword
   %elif regsize3 == 256
      %define sizeptr3 yword
   %elif regsize3 == 512
      %define sizeptr3 zword
   %else
      %error unknown register size 2
   %endif
   %define rc1 sizeptr3 [psi]
   %define rc2 sizeptr3 [rdi]
   %define rc3 sizeptr3 [rsi]
   %define rc4 sizeptr3 [rdi]
%else
   %error unknown register type 3 regtype3
%endif
%endif

; define immediate operands
; (for unknown reasons, this has to be moved inside the testcode macro):
%if 0
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

%if numimm3 == 1
   %define immoperands3 , immvalue3
%else
   %define immoperands3
%endif
%endif

; initialization of registers
%macro testinit2 0
   xor eax, eax
%endmacro

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; test code for round trip latency:
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
%ifidni tmode, L

   %if numinstr == 2
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

         %ifidni instruct2, n
            ; no second instruction, only implicit conversion
            %if numop1 == 2
               instruct1 rb1, ra1 immoperands1
            %elif numop1 == 3
               %if instmix1 == 0
                  instruct1 rb1, ra3, ra1 immoperands1
               %else ; mixed operand types
                  instruct1 rb1, rb3, ra1 immoperands1
               %endif
            %else
               %error wrong number of operands numop1
            %endif
         %else
            ; second instruction is normal
            %if numop1 == 2
               instruct1 rb2, ra1 immoperands1
            %elif numop1 == 3
               instruct1 rb2, ra3, ra1 immoperands1
            %else
               %error wrong number of operands numop1
            %endif
            %if numop2 == 2
               instruct2 ra1, rb2 immoperands2
            %elif numop2 == 3
               %if instmix2 == 0
                  instruct2 ra1, rb4, rb2 immoperands2
               %else  ; mixed source operand types
                  instruct2 ra1, ra4, rb2 immoperands2
               %endif
            %else
               %error wrong number of operands numop2
            %endif
         %endif

      %endmacro

   %elif numinstr == 3
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
         %if numimm3 == 1
            %define immoperands3 , immvalue3
         %else
            %define immoperands3
         %endif

         %ifidni instruct2, n
            ; no second instruction, only implicit conversion
            %if numop1 == 2
               instruct1 rb2, ra1 immoperands1
            %elif numop1 == 3
               %if instmix1 == 0
                  instruct1 rb2, ra1, ra1 immoperands1
               %else  ; mixed source operand types
                  instruct1 rb2, rb1, ra1 immoperands1
               %endif
            %else
               %error wrong number of operands numop1
            %endif
            %if numop3 == 2
               instruct3 ra1, rc2 immoperands3
            %elif numop3 == 3
               %if instmix3 == 0
                  instruct3 ra1, rc3, rc2 immoperands3
               %else
                  instruct3 ra1, ra3, rc2 immoperands3
               %endif
            %else
               %error wrong number of operands numop3
            %endif

         %elifidni instruct3, n
            ; no third instruction, only implicit conversion
            ; all instructions are normal
            %if numop1 == 2
               instruct1 rb2, ra1 immoperands1
            %elif numop1 == 3
               %if instmix1 == 0
                  instruct1 rb2, ra3, ra1 immoperands1
               %else
                  instruct1 rb2, rb3, ra1 immoperands1
               %endif
            %else
               %error wrong number of operands numop1
            %endif
            %if numop2 == 2
               instruct2 rc1, rb2 immoperands2
            %elif numop2 == 3
               %if instmix2 == 0
                  instruct2 rc1, rb4, rb2 immoperands2
               %else
                  instruct2 rc1, rc4, rb2 immoperands2
               %endif
            %else
               %error wrong number of operands numop2
            %endif

         %else
            ; all instructions are normal
            %if numop1 == 2
               instruct1 rb2, ra1 immoperands1
            %elif numop1 == 3
               %if instmix1 == 0               
                  instruct1 rb2, ra1, ra1 immoperands1
               %else
                  instruct1 rb2, rb1, ra1 immoperands1
               %endif
               instruct1 rb2, ra1, ra1 immoperands1
            %else
               %error wrong number of operands numop1
            %endif
            %if numop2 == 2
               instruct2 rc3, rb2 immoperands2
            %elif numop2 == 3
               %if instmix2 == 0
                  instruct2 rc3, rb2, rb2 immoperands2
               %else
                  instruct2 rc3, rc2, rb2 immoperands2
               %endif
            %else
               %error wrong number of operands numop2
            %endif
            %if numop3 == 2
               instruct3 ra1, rc3 immoperands3
            %elif numop3 == 3
               %if instmix3 == 0               
                  instruct3 ra1, rc3, rc3 immoperands3
               %else
                  instruct3 ra1, ra3, rc3 immoperands3
               %endif
            %else
               %error wrong number of operands numop3
            %endif
         %endif
      %endmacro
   %else
      %error wrong number of instructions
   %endif

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; test code for throughput
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
%elifidni tmode, T1

   %macro testcode 0
      %if numimm1 == 1
         %define immoperands1 , immvalue1
      %else
         %define immoperands1
      %endif
      %if numop1 == 2
         instruct1 rb2, ra1 immoperands1
         instruct1 rb4, ra3 immoperands1
      %elif numop1 == 3
         %if instmix1 == 0
            instruct1 rb3, ra2, ra1 immoperands1
            instruct1 rb4, ra2, ra1 immoperands1
         %else ; cannot have two memory operands
            instruct1 rb3, rb2, ra1 immoperands1
            instruct1 rb4, rb2, ra1 immoperands1
         %endif
      %else
         %error wrong number of operands numop1
      %endif
   %endmacro

%elifidni tmode, T2

   %if numop2 == 2
      %macro testcode 0
         %if numimm2 == 1
            %define immoperands2 , immvalue2
         %else
            %define immoperands2
         %endif
         %if numinstr == 2
            instruct2 ra2, rb1 immoperands2
            instruct2 ra4, rb3 immoperands2
         %elif numinstr == 3
            instruct2 rc2, rb1 immoperands2
            instruct2 rc4, rb3 immoperands2
         %endif
      %endmacro
   %elif numop2 == 3
      %macro testcode 0
         %if numimm2 == 1
            %define immoperands2 , immvalue2
         %else
            %define immoperands2
         %endif
         %if numinstr == 2
               %if instmix2 == 0
               instruct2 ra3, rb2, rb1 immoperands2
               instruct2 ra4, rb2, rb1 immoperands2
            %else ; cannot have two memory operands
               instruct2 ra3, ra2, rb1 immoperands2
               instruct2 ra4, ra2, rb1 immoperands2
            %endif
         %elif numinstr == 3
            %if instmix2 == 0
               instruct2 rc3, rb2, rb1 immoperands2
               instruct2 rc4, rb2, rb1 immoperands2
            %else ; cannot have two memory operands
               instruct2 rc3, rc2, rb1 immoperands2
               instruct2 rc4, rc2, rb1 immoperands2
            %endif
         %endif
      %endmacro
   %else
      %error wrong number of operands numop2
   %endif

%elifidni tmode, T3

   %macro testcode 0
      %if numimm3 == 1
         %define immoperands3 , immvalue3
      %else
         %define immoperands3
      %endif
      %if numop3 == 2
         instruct3 ra2, rc1 immoperands3
         instruct3 ra4, rc3 immoperands3
      %elif numop3 == 3
         %if instmix3 == 0
            instruct3 ra3, rc2, rc1 immoperands3
            instruct3 ra4, rc2, rc1 immoperands3
         %else
            instruct3 ra3, ra2, rc1 immoperands3
            instruct3 ra4, ra2, rc1 immoperands3
         %endif
      %else
         %error wrong number of operands numop3
      %endif
   %endmacro

%else
   %error unknown test mode tmode
%endif
