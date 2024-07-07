;----------------------------------------------------------------------------
;                instruct_boundaries.inc                2013-07-11 Agner Fog
;
; PMC Test program for testing if instruction boundaries are marked in instruction cache
;
; (c) Copyright 2013 by Agner Fog. GNU General Public License www.gnu.org/licenses
;-----------------------------------------------------------------------------

; loops in template
%define repeat0 16
%define repeat1  1
%define repeat2  1

%ifndef repeatu
   %define repeatu 100
%endif

%macro testcode 0

   test r14d, 2  ; loop counter
   jnz $+4       ; jump past previous instruction boundary
   
   %rep repeatu
      mov eax,0xB8B8B8B8
      mov ebx,0xBBBBBBBB
   %endrep 
   
   times 4 nop   ; absorb last partial instruction  

%endmacro
