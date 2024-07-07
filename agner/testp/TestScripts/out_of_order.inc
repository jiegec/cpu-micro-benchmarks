;----------------------------------------------------------------------------
;                out_of_order.inc                          2014-08-03 Agner Fog
;
; PMC Test program for measuring out-of-order capacity
;
; (c) 2014 GNU General Public License www.gnu.org/licenses
;
; Test cases:
; 1: integer add
; 2: integer mul
; 3: floating point add
; 4: floating point mul
;-----------------------------------------------------------------------------
; Define any undefined macros

; total number of instructions
%ifndef totalrep
   %define totalrep 1000000
%endif

%if chainlength < totalrep
   %define repeat1 (totalrep / chainlength / 2)
   %define clength chainlength
%else
   %define repeat1 (totalrep / 100)
   %define clength 100
%endif


; Define test cases

%if tcase == 1   ; integer add

   %macro chain1 0
      %rep clength
         add eax,eax
      %endrep
   %endmacro

   %macro chain2 0
      %rep clength
         add ebx,ebx
      %endrep
   %endmacro

%elif tcase == 2   ; integer mul

   %macro chain1 0
      %rep clength
         imul eax,eax
      %endrep
   %endmacro

   %macro chain2 0
      %rep clength
         imul ebx,ebx
      %endrep
   %endmacro

%elif tcase == 3   ; floating point add

   %macro chain1 0
      %rep clength
         addps xmm1,xmm1
      %endrep
   %endmacro

   %macro chain2 0
      %rep clength
         addps xmm2,xmm2
      %endrep
   %endmacro

%elif tcase == 4   ; floating point mul

   %macro chain1 0
      %rep clength
         mulps xmm1,xmm1
      %endrep
   %endmacro

   %macro chain2 0
      %rep clength
         mulps xmm2,xmm2
      %endrep
   %endmacro

%else
   %error unknown test case tcase
%endif


%macro testcode 0
   %if chainlength < totalrep
      chain1
      chain2
   %else
      chain1
   %endif
%endmacro


; disable default test loops
%define repeat2 1

