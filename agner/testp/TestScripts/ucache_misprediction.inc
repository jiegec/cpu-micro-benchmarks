;----------------------------------------------------------------------------
;                       ucache_misprediction.inc        2013-07-21 Agner Fog
;
;            PMC Test program for testingbranch prediction
;                           NASM syntax
;
; The following macros can be defined on the command line or in include files:
; 
; tcase:    Test case number. See below for each case
;           1. Tiny loop. Expect loop counter to be used
;           2. Normal loop. Expect uop cache to be used
;           3. Extremely big loop. Expect only fetch and decode to be used
; 
; count1:   Loop count for outer loop
; 
; count2:   Loop count for inner loop
;
;
; (c) Copyright 2013 by Agner Fog. GNU General Public License www.gnu.org/licenses
;-----------------------------------------------------------------------------

%ifndef tcase
   %define case  1          ; default case 1
%endif

%ifndef count1
   %define count1   10      ; default count1
%endif

%ifndef count2
   %define count2   16      ; default count2
%endif


;##############################################################################
;#
;#                 Test code macros
;#
;##############################################################################

; define long nops
%ifndef noptype
   %define noptype 2
%endif

%include "nops.inc"

%if tcase == 1   ; Tiny loop. Expect loop counter to be used

%macro testcode 0
nop
nop
mov ebp, count1
align 16
LL:
    test r14b,4
    jz L2
    nop8
    L2:
    nop8
dec ebp
jnz LL
%endmacro

%elif tcase == 2 ; Normal loop. Expect uop cache to be used

%macro testcode 0
%rep count1
    test r14b,4
    jnz $+10
    nop8
    nop8
%endrep
%endmacro

%elif tcase == 3 ; Extremely big loop. Expect only fetch and decode to be used

%macro testinitc 0
%rep 100000    ; lots of code before counters are read, to prevent uop caching
nop
%endrep
%endmacro

%macro testcode 0
%rep count1
    test r14b,4
    jnz $+10
    nop8
    nop8
%endrep
%endmacro

%else
%error unknown test case tcase
%endif

; disable default test loops
%define repeat1 1
%define repeat2 1

