; unaligned_mem.inc                                                        2014-01-31 AgF

; Test unaligned memory read and write throughput
; (c) 2014 by Agner Fog. GNU General Public License www.gnu.org/licenses

; Parameters:
;
; regsize:   size of read or write operand, default = 32
; roffset:   offset to aligned boundary, default = 0
; alignment: alignment of boundary to cross
; tmode:     R: read, W: write, WR: write, then read (store forwarding)

%ifndef regsize
   %define roffset 32
%endif

%ifndef roffset
   %define roffset 0
%endif

%ifndef alignment
   %define alignment 64
%endif

%ifndef tmode
   %define tmode R
%endif

; define move instruction
%if regsize < 65  
   %define moveinst  mov
%elif regsize == 65  
   %define moveinst  movq
%elif regsize == 128  
   %define moveinst  movdqu
%elif regsize == 256  
   %define moveinst  vmovdqu
%else
   %error unknown register size
%endif


; initialization of aligned or misaligned pointer
%macro testinit1 0
    lea psi, [UserData + 1000h]
    and psi, -1000h
%endmacro


; main testcode macro
%macro testcode 0
   %ifidni tmode, R
      moveinst reg0, [psi+alignment-roffset]
   %elifidni tmode, W
      moveinst [psi+alignment-roffset], reg0
   %elifidni tmode, WR
      moveinst [psi+alignment-roffset], reg0
      moveinst reg0, [psi-roffset]
   %endif
%endmacro


%macro testdata 0
        times 10000H  DB 0
%endmacro


; test loops
%define repeat1 1000
%define repeat2 100
