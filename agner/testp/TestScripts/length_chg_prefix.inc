;----------------------------------------------------------------------------
;                length_chg_prefix.inc                2013-07-10 Agner Fog
;
; PMC Test program for testing decode penalties on length-changing prefixes
;
; Constants to be defined:
; 
; tcase:   1: mov register,constant
;          2: add register,constant, etc.
;          3: test register,constant
;          4: neg or not register (bogus length-changing prefix)
;          5: lea with address size prefix (must run in 32-bit mode)
;
; tmode:   1: instructions with length-changing prefix, aligned by 16
;          2: instructions with length-changing prefix, crossing 16-bytes boundary
;          3: similar instructions with non-length-changing prefix
; 
; (c) Copyright 2013 by Agner Fog. GNU General Public License www.gnu.org/licenses
;-----------------------------------------------------------------------------
; Define any undefined macros

%ifndef tcase
   %define tcase 1
%endif

%ifndef tcase
   %define tmode 1
%endif

; define long nops
%ifndef noptype
   %define noptype 2
%endif

%include "nops.inc"


; Define test cases

%if tcase == 1   ; mov register,constant

   %if tmode < 3      ; instructions with length-changing prefix

      %macro testcode 0
         %if tmode == 2
            nop2          ; misalign
         %endif
         %rep 250         
            mov ax,1111h      ; 4 bytes
            mov bx,2222h      ; 4 bytes
            mov cx,3333h      ; 4 bytes
            mov dx,4444h      ; 4 bytes
         %endrep
      %endmacro

   %elif tmode == 3   ; similar instructions with non-length-changing prefix
   
      %macro testcode 0
         %rep 250         
            db 3eh             ; 1 byte. dummy segment prefix
            mov eax,1111h      ; 5 bytes
            mov ebx,2222h      ; 5 bytes
            db 3eh             ; 1 byte. dummy segment prefix
            mov cl,33h         ; 2 bytes
            mov dl,44h         ; 2 bytes
         %endrep
      %endmacro
   
   %endif

%elif tcase == 2   ; add register,constant, etc.

   %if tmode < 3      ; instructions with length-changing prefix

      %macro testcode 0
         %if tmode == 2
            nop4             ; misalign
         %endif
         %rep 250         
            add ax,1111h     ; 5 bytes
            add bx,2222h     ; 5 bytes
            add cx,3333h     ; 5 bytes
            nop              ; 1 byte
         %endrep
      %endmacro

   %elif tmode == 3   ; similar instructions with non-length-changing prefix
   
      %macro testcode 0
         %rep 250         
            add ax,1         ; 4 bytes
            add bx,2         ; 4 bytes
            add cx,3         ; 4 bytes
            nop4             ; 4 bytes
         %endrep
      %endmacro
   
   %endif

%elif tcase == 3   ; test register,constant

   %if tmode < 3      ; instructions with length-changing prefix

      %macro testcode 0
         %if tmode == 2
            nop4              ; misalign
         %endif
         %rep 250         
            test ax,1111h     ; 5 bytes
            test bx,2222h     ; 5 bytes
            test cx,3333h     ; 5 bytes
            nop               ; 1 byte
         %endrep
      %endmacro

   %elif tmode == 3   ; similar instructions with non-length-changing prefix
   
      %macro testcode 0
         %rep 250         
            cmp ax,1         ; 3 bytes
            cmp bx,2         ; 3 bytes
            cmp cx,3         ; 3 bytes
            nop7             ; 7 bytes
         %endrep
      %endmacro
   
   %endif

%elif tcase == 4   ; neg or not register (bogus length-changing prefix)

   %if tmode < 3      ; instructions with length-changing prefix

      %macro testcode 0
         %if tmode == 2
            nop              ; misalign
         %endif
         %rep 250         
            neg ax           ; 3 bytes
            neg bx           ; 3 bytes
            not cx           ; 3 bytes
            not dx           ; 3 bytes
         %endrep
      %endmacro

   %elif tmode == 3   ; similar instructions with non-length-changing prefix
   
      %macro testcode 0
         %rep 250         
            cmp ax,1         ; 3 bytes
            cmp bx,2         ; 3 bytes
            nop7             ; 7 bytes
            cmp cx,3         ; 3 bytes
         %endrep
      %endmacro
   
   %endif

%elif tcase == 5   ; lea with address size prefix

   %if tmode < 3      ; instructions with length-changing prefix

      %macro testcode 0
         %if tmode == 2
            nop3               ; misalign
         %endif
         %rep 250         
            lea eax,[si+1]     ; 4 bytes (would have sib byte without address size prefix)
            lea edx,[bx+2222h] ; 5 bytes (would have two more displacement bytes without address size prefix)
            nop2               ; 2 bytes
            lea edi,[si+3333h] ; 5 bytes (both effects)
         %endrep
      %endmacro

   %elif tmode == 3   ; similar instructions with non-length-changing prefix
   
      %macro testcode 0
         %rep 250         
            lea eax,[bx+1]     ; 4 bytes
            lea cx, [bx+1]     ; 5 bytes
            nop2               ; 2 bytes
            lea dx, [bx+1]     ; 5 bytes
         %endrep
      %endmacro
   
   %endif

%else
   %error unknown test case tcase
%endif

; disable default test loops
%define repeat1 1000
%define repeat2 1
