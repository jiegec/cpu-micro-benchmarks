; read_write_bandwidth.inc                         2021-03-23 Agner Fog

; Test the maximum number of memory reads and writes per clock cycle
;
; (c) 2013-2021 by Agner Fog. GNU General Public License www.gnu.org/licenses
;
; Parameters:
;
; tmode:       Test mode:
;              R:    read only
;              W:    write only
;              RW:   one read and one write
;              RRW:  two reads and one write
;              RWW:  one read and two writes
;              RWW2: one read and two writes to different cache lines
;
; regsize:     register size, bits

; define appropriate move instruction:
%if regsize <= 64
   %define mov1 mov
%elif regsize == 65    ; 64 bit mmx registers
   %define mov1 movq
%elif regsize == 128
   %define mov1 movdqa
%elif regsize == 256
   %define mov1 vmovdqa
%elif regsize == 512
   %define mov1 vmovdqa64
%else
   %error unknown register size regsize
%endif


; main testcode macro
%macro testcode 0

   %ifidni tmode, R  ; read only
   
      %rep 25
         mov1 reg0, [psi]
         mov1 reg1, [psi+regsize/8]
         mov1 reg2, [psi+2*regsize/8]
         mov1 reg3, [psi+3*regsize/8]
      %endrep   
         
   %elifidni tmode, W  ; write only
   
      %rep 25
         mov1 [psi], reg0
         mov1 [psi+regsize/8], reg1
         mov1 [psi+2*regsize/8], reg2
         mov1 [psi+3*regsize/8], reg3
      %endrep   
         
   %elifidni tmode, RW  ; one read and one write
   
      %rep 50
         mov1 reg0, [psi]
         mov1 [psi+regsize/8], reg1
         mov1 reg2, [psi+2*regsize/8]
         mov1 [psi+3*regsize/8], reg3
      %endrep   

   %elifidni tmode, RRW  ; two reads and one write
   
      %rep 50
         mov1 reg0, [psi]
         mov1 reg1, [psi+regsize/8]
         mov1 [psi+4*regsize/8], reg2
         mov1 reg3, [psi+2*regsize/8]
         mov1 reg4, [psi+3*regsize/8]
         mov1 [psi+5*regsize/8], reg5
      %endrep 

   %elifidni tmode, RRRW  ; three reads and one write
   
      %rep 50
         mov1 reg0, [psi]
         mov1 reg1, [psi+2*regsize/8]
         mov1 reg2, [psi+4*regsize/8]
         mov1 [psi+6*regsize/8], reg2
      %endrep     

   %elifidni tmode, RWW  ; one read and two writes
   
      %rep 50
         mov1 reg0, [psi]
         mov1 [psi+2*regsize/8], reg1
         mov1 [psi+3*regsize/8], reg2
         mov1 reg3, [psi+1*regsize/8]
         mov1 [psi+4*regsize/8], reg4
         mov1 [psi+5*regsize/8], reg5
      %endrep
      
   %elifidni tmode, RWW2  ; one read and two writes to different cache lines
   
      %rep 50
         mov1 reg0, [psi]
         mov1 [psi+0x40], reg1
         mov1 [psi+0x80], reg2
         mov1 reg3, [psi+1*regsize/8]
         mov1 [psi+0xC0], reg4
         mov1 [psi+0x100], reg5
      %endrep   
    
%else 
  
    %error unknown test mode tmode
    
%endif
    
%endmacro

; test loops
%define repeat1 1000
%define repeat2 1
