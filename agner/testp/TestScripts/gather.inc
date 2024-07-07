; convers2.inc                                       2013-07-08 Agner Fog
;
; Define test code to test latency and throughput for AVX2 gather instructions
; (c) Copyright 2013 by Agner Fog. GNU General Public License www.gnu.org/licenses
;
; Parameters:
;
; instruct:    Instruction to test. Must be a gather instruction
;
; regsize:     Size of destination register operand. 128 or 256
;
; iregsize:    Size of index register. 128 or 256
;
; tmode:       Test mode:
;
;              NONE:         Do nothing. Mask = 0
;
;              ONE:          Load only one data item
;
;              CONTIGUOUS:   Load contiguous data items
;
;              STRIDE:       Load data items with a stride of 4 items
;
;              RANDOM:       Load items in no particular order
;
;              SAME:         Some items are the same. Use as shuffle
;
;              PART_OVERLAP: Data items are partially overlapping
;
;              LATENCY:      Load + store latency
;
;              LATENCY_I:    Load + VPOR + store latency
;
;              LATENCY_F:    Load + VMAXPS + store latency
;
; -------------------------------------------------------------------------------------------------

%define repeat1  1000        ; loop in TemplateB64.nasm
%define repeat2  1           ; disable loop in TemplateB64.nasm

%ifndef regsize
  %define regsize 128
%endif

; Get parameters from instruction

%ifidni instruct,vgatherdps
   %define elementsize 4          ; size of data elements, bytes
   %define indexsize 4            ; size of index vector elements, bytes
   %define moveinst vmovaps       ; best move instruction for data type
%elifidni instruct,vgatherqps
   %define elementsize 4
   %define indexsize 8
   %define moveinst vmovaps
%elifidni instruct,vgatherdpd
   %define elementsize 8
   %define indexsize 4
   %define moveinst vmovapd
%elifidni instruct,vgatherqpd 
   %define elementsize 8
   %define indexsize 8
   %define moveinst vmovapd
%elifidni instruct,vpgatherdd
   %define elementsize 4
   %define indexsize 4
   %define moveinst vmovdqa
%elifidni instruct,vpgatherqd
   %define elementsize 4
   %define indexsize 8
   %define moveinst vmovdqa
%elifidni instruct,vpgatherdq
   %define elementsize 8
   %define indexsize 4
   %define moveinst vmovdqa
%elifidni instruct,vpgatherqq
   %define elementsize 8
   %define indexsize 8
   %define moveinst vmovdqa
%else
   %error unknown instruction instruct
%endif

; Define test data
%macro testdata 0
   %if indexsize == 32
      %define datadef   dd
   %else  ; 64
      %define datadef   dq
   %endif

   Indexdata:
   %ifidni tmode,NONE   ; Do nothing. Mask = 0
      datadef  0,0,0,0,0,0,0,0
      %define  scale  elementsize

   %elifidni tmode,ONE   ; Load only one element
      datadef  0,0,0,0,0,0,0,0
      %define  scale  elementsize
      
   %elifidni tmode,CONTIGUOUS   ; Load contiguous data items
      datadef  0,1,2,3,4,5,6,7
      %define  scale  elementsize

   %elifidni tmode,STRIDE       ; Load data items with a stride of 4 items
      datadef  0*2,4*2,8*2,12*2,16*2,20*2,24*2,28*2
      %define  scale  elementsize/2

   %elifidni tmode,RANDOM       ; Load items in no particular order
      datadef  5,4,21,10,1,8,11,15
      %define  scale  elementsize

   %elifidni tmode,SAME         ; Some items are the same. Use as shuffle
      datadef  0,0,2,2,0,0,2,2
      %define  scale  elementsize

   %elifidni tmode,PART_OVERLAP ; Data items are partially overlapping
      datadef  0,elementsize/2,16,16+elementsize/2,32,32+elementsize/4,32+elementsize/2,32+elementsize*3/4
      %define  scale  1

   %elifidni tmode,LATENCY      ; Load + store latency
      datadef  0,2,4,6,8,10,12,14
      %define  scale  elementsize

   %elifidni tmode,LATENCY_I    ; Load + VPOR + store latency
      datadef  0,2,4,6,8,10,12,14
      %define  scale  elementsize

   %elifidni tmode,LATENCY_F    ; Load + VMAXPS + store latency
      datadef  0,2,4,6,8,10,12,14
      %define  scale  elementsize
      
   %else
      %error unknown test mode tmode
   %endif
   
   Loaddata:                     ; Data for loading
   times 1000H  datadef 0
   
%endmacro

; Get index register
%if iregsize == 128
   %define indexreg xmm6
%else
   %define indexreg ymm6
%endif 

; Initialize index and mask and pointer
%macro testinit1  0
   vmovdqa indexreg, [Indexdata]    ; ymm6 = index
   %ifidni tmode,NONE   ; Mask = 0
      vpxor ymm7,ymm7           ; ymm7 = mask
   %else                ; Mask = FF   
      vpcmpeqd ymm7,ymm7
   %endif
   lea rsi, [Loaddata]          ; rsi = base pointer
%endmacro
   

; Test code

%ifidni tmode,LATENCY      ; Load + store latency

   %macro testcode 0
      %rep 100
      vmovdqa reg2, reg7    ; mask      
      instruct reg0, [rsi + scale*indexreg], reg2
      moveinst [rsi], reg0
      %endrep
   %endmacro

%elifidni tmode,LATENCY_I    ; Load + VPOR + store latency

   %macro testcode 0
      %rep 100
      vmovdqa reg2, reg7    ; mask      
      instruct reg0, [rsi + scale*indexreg], reg2
      vpor reg0, reg0, reg0
      moveinst [rsi], reg0
      %endrep
   %endmacro
   
%elifidni tmode,LATENCY_F    ; Load + VMAXPS + store latency
   
   %macro testcode 0
      %rep 100
      vmovdqa reg2, reg7    ; mask      
      instruct reg0, [rsi + scale*indexreg], reg2
      vmaxps reg0, reg0, reg0
      moveinst [rsi], reg0
      %endrep
   %endmacro
   
%else        ; any other tmode: measure throughput only

   %macro testcode 0
      %rep 50
      vmovdqa reg2, reg7    ; mask      
      instruct reg0, [rsi + scale*indexreg], reg2
      vmovdqa reg2, reg7    ; mask      
      instruct reg0, [rsi + 0x100 + scale*indexreg], reg2
      %endrep
   %endmacro

%endif
