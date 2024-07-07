; cache_banks.inc                                       2015-12-20 Agner Fog

; Measure cache bank conflicts
; (c) 2014 - 2015 by Agner Fog. GNU General Public License www.gnu.org/licenses
;
; Parameters:
;
; stride1:     Bigger than all banks
;
; stride2:     Bank size
;
; tmode:       Test mode:
;              1:    read from two addresses spaced by stride1
;              2:    read from two addresses spaced by stride1 + stride2
;              3:    read and write from two addresses spaced by stride1
;              4:    read and write from two addresses spaced by stride1 + stride2

%macro testinit1 0

%endmacro


; main testcode macro

%macro testcode 0
%if tmode == 1    ; 
    mov eax,[rsi]
    mov ebx,[rsi + stride1]
%elif tmode == 2    ; 
    mov eax,[rsi]
    mov ebx,[rsi + stride1 + stride2]
%elif tmode == 3    ; 
    mov eax,[rsi]
    mov [rsi + stride1],eax
%elif tmode == 4    ; 
    mov eax,[rsi]
    mov [rsi + stride1 + stride2],eax
%endif
%endmacro
