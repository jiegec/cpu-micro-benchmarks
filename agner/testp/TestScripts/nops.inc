;----------------------------------------------------------------------------
;                       nops.inc                      2013-08-11 Agner Fog
;
; Include file defining long nops
;
; The following macros can be defined on the command line or in include files:
; 
; noptype:      1: pseudo-NOPs such as LEA eax,[eax+00]
;               2: long NOPs (0F 1F ...)
;               3: 66 NOPs (simple NOP with up to 14 operand size prefixes)
;
; (c) Copyright 2013 by Agner Fog. GNU General Public License www.gnu.org/licenses
;-----------------------------------------------------------------------------

; Define noptype if not defined

%ifndef noptype
   %define noptype 2
%endif

; define nops of all lengths

%if noptype == 1             ; pseudo-NOPs such as LEA eax,[eax+0]
   %define nop1  db 90h                                 ; nop
   %define nop2  db 89H, 0C0H                           ; mov eax,eax
   %define nop3  db 8DH, 40H, 00H                       ; lea eax,[eax+00]
   %define nop4  db 8DH, 64H, 24H, 00H                  ; lea esp,[esp+00]
   %define nop5  db 3EH, 8DH, 64H, 24H, 00H             ; lea esp,[ds:esp+00]
   %define nop6  db 8DH, 80H, 00H, 00H, 00H, 00H        ; lea eax,[eax+00000000]
   %define nop7  db 8DH, 0A4H, 24H, 00H, 00H, 00H, 00H  ; lea esp,[esp+00000000]
   %define nop8  db 3EH, 8DH, 0A4H, 24H, 00H, 00H, 00H, 00H ; lea esp,[ds:esp+00000000]
   %define nop9  db 3EH, 3EH, 8DH, 0A4H, 24H, 00H, 00H, 00H, 00H ; lea esp,[ds:esp+00000000]
   %define nop10 db 3EH, 3EH, 3EH, 8DH, 0A4H, 24H, 00H, 00H, 00H, 00H ; lea esp,[ds:esp+00000000]
   %define nop11 db 3EH, 3EH, 3EH, 3EH, 8DH, 0A4H, 24H, 00H, 00H, 00H, 00H ; lea esp,[ds:esp+00000000]
   %define nop12 db 3EH, 3EH, 3EH, 3EH, 3EH, 8DH, 0A4H, 24H, 00H, 00H, 00H, 00H ; lea esp,[ds:esp+00000000]
   %define nop13 db 3EH, 3EH, 3EH, 3EH, 3EH, 3EH, 8DH, 0A4H, 24H, 00H, 00H, 00H, 00H ; lea esp,[ds:esp+00000000]
   %define nop14 db 3EH, 3EH, 3EH, 3EH, 3EH, 3EH, 3EH, 8DH, 0A4H, 24H, 00H, 00H, 00H, 00H ; lea esp,[ds:esp+00000000]
   %define nop15 db 3EH, 3EH, 3EH, 3EH, 3EH, 3EH, 3EH, 3EH, 8DH, 0A4H, 24H, 00H, 00H, 00H, 00H ; lea esp,[ds:esp+00000000]

%elif noptype == 2         ; long NOPs
   %define nop1  db 90H
   %define nop2  db 66H, 90H
   %define nop3  db 0FH, 1FH, 0C0H
   %define nop4  db 0FH, 1FH, 40H, 00H
   %define nop5  db 0FH, 1FH, 44H, 00H, 00H
   %define nop6  db 66H, 0FH, 1FH, 44H, 00H, 00H
   %define nop7  db 0FH, 1FH, 80H, 00H, 00H, 00H, 00H
   %define nop8  db 0FH, 1FH, 84H, 00H, 00H, 00H, 00H, 00H
   %define nop9  db 66H, 0FH, 1FH, 84H, 00H, 00H, 00H, 00H, 00H
   %define nop10 db 66H, 66H, 0FH, 1FH, 84H, 00H, 00H, 00H, 00H, 00H
   %define nop11 db 66H, 66H, 66H, 0FH, 1FH, 84H, 00H, 00H, 00H, 00H, 00H
   %define nop12 db 66H, 66H, 66H, 66H, 0FH, 1FH, 84H, 00H, 00H, 00H, 00H, 00H
   %define nop13 db 66H, 66H, 66H, 66H, 66H, 0FH, 1FH, 84H, 00H, 00H, 00H, 00H, 00H
   %define nop14 db 66H, 66H, 66H, 66H, 66H, 66H, 0FH, 1FH, 84H, 00H, 00H, 00H, 00H, 00H
   %define nop15 db 66H, 66H, 66H, 66H, 66H, 66H, 66H, 0FH, 1FH, 84H, 00H, 00H, 00H, 00H, 00H

%elif noptype == 3          ; 66 prefix NOPs
   %define nop1  db 90H
   %define nop2  db 66H, 90H
   %define nop3  db 66H, 66H, 90H
   %define nop4  db 66H, 66H, 66H, 90H
   %define nop5  db 66H, 66H, 66H, 66H, 90H
   %define nop6  db 66H, 66H, 66H, 66H, 66H, 90H
   %define nop7  db 66H, 66H, 66H, 66H, 66H, 66H, 90H
   %define nop8  db 66H, 66H, 66H, 66H, 66H, 66H, 66H, 90H
   %define nop9  db 66H, 66H, 66H, 66H, 66H, 66H, 66H, 66H, 90H
   %define nop10 db 66H, 66H, 66H, 66H, 66H, 66H, 66H, 66H, 66H, 90H
   %define nop11 db 66H, 66H, 66H, 66H, 66H, 66H, 66H, 66H, 66H, 66H, 90H
   %define nop12 db 66H, 66H, 66H, 66H, 66H, 66H, 66H, 66H, 66H, 66H, 66H, 90H
   %define nop13 db 66H, 66H, 66H, 66H, 66H, 66H, 66H, 66H, 66H, 66H, 66H, 66H, 90H
   %define nop14 db 66H, 66H, 66H, 66H, 66H, 66H, 66H, 66H, 66H, 66H, 66H, 66H, 66H, 90H
   %define nop15 db 66H, 66H, 66H, 66H, 66H, 66H, 66H, 66H, 66H, 66H, 66H, 66H, 66H, 66H, 90H

%else

   ; %error unknown nop type noptype

%endif
