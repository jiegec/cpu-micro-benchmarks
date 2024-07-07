; returnstack.inc                                                                 2012-02-22 AgF

; Test return stack buffer
; (c) 2012 by Agner Fog. GNU General Public License www.gnu.org/licenses

; Parameters:
;
; nesting: function nesting level (minimum value = 2)

%ifndef nesting
   %define nesting 8
%endif

; define function named F%1 calling F%2
%macro definefunc 2
   F%1: nop
   nop
   nop
   nop
   call F%2
   nop
   nop
   nop
   nop
   ret
   nop
   nop
%endmacro

; main testcode macro
%macro testcode 0
   jmp ZZZ
   nop
   nop

   align 16
   ; define nested functions
   %assign n nesting-1
   %if n < 1
      %assign n 1
   %endif

   %assign i 0
   %rep n
      %assign i i+1
      %assign j i+1
      %if j > n
         %assign j 9999
      %endif
      definefunc i, j
   %endrep

   ; Last function
   F9999:
      nop
      nop
      ret

   align 16
   ZZZ: nop
      nop
      call F1
      nop
      nop
      call F1
      nop
      nop

%endmacro

; disable default test loops
%define repeat1 100
%define repeat2 1

