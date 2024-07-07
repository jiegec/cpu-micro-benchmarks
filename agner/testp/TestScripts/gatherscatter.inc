; gatherscatter.inc                                       2016-10-11 Agner Fog
;
; Define test code to test latency and throughput for gather and scatter instructions
; (c) Copyright 2013-2016 by Agner Fog. GNU General Public License www.gnu.org/licenses
;
; Parameters:
;
; instruct:    Instruction to test. Must be a gather or scatter instruction
;
; scatter:     1 if scatter instruction, 0 if gather
;
; regsize:     Size of data register. 128, 256 or 512
;
; iregsize:    Size of index register. 128, 256 or 512
;
; datasize:    Data element size. 32 or 64
;
; indexsize:   Index element size. 32 or 64
;
; masktype:    Register type for status mask: k for mask register, v for vector register
;
; mask:        Initial value of mask. Default = all 1's
;
; numop:       Number of register, memory and mask operands (3 for gather and scatter, 2 for prefetch)
;
; tmode:       Test mode:
;
;              ONE:          Load only one data item
;
;              CONTIGUOUS:   Load contiguous data items
;
;              STRIDE:       Load data items with a stride of 4 items
;
;              RANDOM:       Load items in random order. (seeded with the time)
;
;              SAME:         Some items are the same. (permuting data)
;
;              PART_OVERLAP: Data items are partially overlapping
;
;              LATENCY:      Gather + store latency, or scatter + load latency. All items on same address
;                            You may set mask = 1 to measure latency of a single element
;
; The repeat count is fixed at 10000
; -------------------------------------------------------------------------------------------------

%define repeat1  100         ; loop in TemplateB64.nasm
%define repeat2  1           ; disable loop in TemplateB64.nasm

; Default parameters
%ifndef scatter
   %define scatter 0
%endif

%ifndef numop
   %define numop 3
%elifempty numop
   %define numop 3
%endif

%ifndef mask
   %define mask -1
%elifempty mask
   %define mask -1
%endif

%ifndef iregsize
   %define iregsize regsize
%endif
%if iregsize == 128
   %define regi xmm5
%elif iregsize == 256
   %define regi ymm5
%elif iregsize == 512
   %define regi zmm5
%else
   %error register size regi not defined
%endif

; Calculate number of elements
%define delements (regsize/datasize)
%define ielements (iregsize/indexsize)
%if delements < ielements
   %define numelements delements
%else
   %define numelements ielements
%endif

; Scale factor
%ifidni tmode, PART_OVERLAP ; Data items are partially overlapping
   %define scale (datasize/4)
%else
   %define scale (datasize/8)
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
      %rep numelements
         datadef  0
      %endrep

   %elifidni tmode,ONE   ; Load only one element
      %rep numelements
         datadef  0
      %endrep
      
   %elifidni tmode,CONTIGUOUS   ; Load contiguous data items: 0, 1, 2, 3, ...
      %assign ii 0
      %rep numelements
         datadef  ii
         %assign ii (ii + 1)
      %endrep

   %elifidni tmode,STRIDE       ; Load data items with a stride of 4 items
      %assign ii 0
      %rep numelements
         datadef  ii*4
         %assign ii (ii + 1)
      %endrep

   %elifidni tmode,RANDOM       ; Load items in random order
      %assign rr __POSIX_TIME__       ; Random number seed
      %rep numelements
         %assign rr (rr ^ (rr << 13))
         %assign rr (rr ^ (rr >> 17))
         %assign rr (rr ^ (rr <<  5))
         datadef  (rr & 0x3F)
      %endrep

   %elifidni tmode,SAME         ; Some items are the same. Permutation
      %assign ii 0
      %rep numelements
         %if numelements < 4
            datadef  0
         %elif numelements < 8
            datadef  ii & 1
         %else
            datadef  (ii & 3) ^ 1
         %endif
         %assign ii (ii + 1)
      %endrep

   %elifidni tmode,PART_OVERLAP ; Data items are partially overlapping
      %assign ii 0
      %rep numelements
         datadef  ii
         %assign ii (ii + 1)
      %endrep

   %elifidni tmode,LATENCY   ; Gather or scatter latency
      %rep numelements
         datadef  0
      %endrep

   %else
      %error unknown test mode tmode
   %endif
   
   Loaddata:                     ; Data for loading
   times 1000H  datadef 0
   
%endmacro

; Initialize index and mask and pointer
%macro testinit1  0
   %if iregsize > 256
      vmovdqu64 regi, [Indexdata]    ; zmm5 = index
   %else
      vmovdqu   regi, [Indexdata]    ; x/ymm5 = index
   %endif
   %ifidni masktype,k                ; use mask register 6 and 7
      %ifidni tmode, NONE   ; Mask = 0
         xor eax, eax
      %elifidni tmode, ONE   ; Mask = 1
         mov eax, 1
      %else
         mov rax, mask
      %endif
      %if numelements < 17
         kmovw k7, eax
      %elif numelements < 33
         kmovd k7, eax
      %elif numelements < 65
         kmovq k7, rax
      %else
         %error Too many elements for mask register
      %endif
   %elifidni masktype,v               ; use vector register 6 and 7
      %ifidni tmode, NONE             ; Mask = 0
         vpxor reg7, reg7
      %elif mask == -1                ; Mask = FF   
         vpcmpeqd reg7, reg7
      %else                           ; Arbitrary mask
         vpxor reg7, reg7
         or rax, -1
         %assign jj 0
         %rep numelements
            %if (mask >> jj) & 1
               %if indexsize == 32
                  vpinsrd reg7, reg7, eax, jj
               %else
                  vpinsrq reg7, reg7, rax, jj
               %endif
            %endif
            %assign jj (jj + 1)
         %endrep
      %endif
   %else
      %error Unknown mask type
   %endif
   lea rsi, [Loaddata]          ; rsi = base pointer
   lea rdi, [rsi+0x400]         ; rdi = second base pointer
%endmacro

%macro resetmask 0  ; reset status mask
   %ifidni masktype, k   
      %if numelements < 17
         kmovw k6, k7
      %elif numelements < 33
         kmovd k6, k7
      %elif numelements < 65
         kmovq k6, k7
      %else
         %error Too many elements for mask register
      %endif
   %else
      vmovdqa reg6, reg7    ; mask register has same size as data register
   %endif
%endmacro


; Test code

%ifidni tmode, LATENCY      ; Load + store latency

   %macro testcode 0
      %if scatter == 1       ; scatter instruction
         %rep 100
            resetmask
            instruct [rsi + scale*regi] {k6}, reg0
            %if datasize == 32
               vmovd xmm0, [rsi]
            %elif datasize == 64
               vmovq xmm0, [rsi]
            %else
               %error unknown data size datasize
            %endif
         %endrep

      %elifidni masktype, k  ; gather with status in mask register
         %rep 100
            resetmask
            instruct reg0 {k6}, [rsi + scale*regi]
            %if datasize == 32
               vmovd [rsi], xmm0
            %elif datasize == 64
               vmovq [rsi], xmm0
            %else
               %error unknown data size datasize
            %endif
         %endrep

      %else                  ; gather with status in vector register
         %rep 100
            resetmask
            instruct reg0, [rsi + scale*regi], reg6
            %if datasize == 32
               vmovd [rsi], reg0
            %elif datasize == 64
               vmovq [rsi], reg0
            %else
               %error unknown data size datasize
            %endif
         %endrep

      %endif
   %endmacro
   
%else        ; any other tmode: measure throughput only

   %macro testcode 0
      %if scatter == 1       ; scatter instruction
         %if numop == 3
            %rep 50
               resetmask
               instruct [rsi + scale*regi] {k6}, reg0
               resetmask
               instruct [rdi + scale*regi] {k6}, reg1
            %endrep
         %else
            %rep 50
               resetmask
               instruct [rsi + scale*regi] {k6}
               resetmask
               instruct [rdi + scale*regi] {k6}
            %endrep
         %endif
      %elifidni masktype, k  ; gather with status in mask register
         %if numop == 3
            %rep 50
               resetmask
               instruct reg0{k6}, [rsi + scale*regi]
               resetmask
               instruct reg1{k6}, [rdi + scale*regi]
            %endrep
         %else
            %rep 50
               resetmask
               instruct [rsi + scale*regi]{k6}
               resetmask
               instruct [rdi + scale*regi]{k6}
            %endrep
         %endif

      %else                  ; gather with status in vector register
         %rep 50
            resetmask
            instruct reg0, [rsi + scale*regi], reg6
            resetmask
            instruct reg1, [rdi + scale*regi], reg6
         %endrep

      %endif
   %endmacro

%endif
