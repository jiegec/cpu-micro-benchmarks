; mov_rename.inc                                           2022-11-02 Agner Fog

; Test if move instructions can be eliminated by renaming, measured in dependency chain

; (c) 2013-2022 by Agner Fog. GNU General Public License www.gnu.org/licenses

; Parameters:
;
; regsize:   Size of register operand, default = 32
; instruct1: Instruction between moves. must have two operands, 'none' for none, default is ADD
; nummov:    Number of move instructions between instruct1
; tcase:     Test case: 1 = same register, 2 = different register, 3 = register known to be zero

%ifndef regsize
   %define regsize 32
%endif

%ifndef nummov
   %define nummov 1
%endif

%ifndef tcase
   %define tcase 2
%endif

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
   %define mov1 vmovdqa32
%else
   %error unknown register size regsize
%endif

; define appropriate add instruction:
%ifndef instruct1
%if regsize <= 64
   %define instruct1 add
%elif regsize == 65    ; 64 bit mmx registers
   %define instruct1 paddw
%elif regsize == 128
   %define instruct1 paddw
%elif regsize == 256
   %define instruct1 vpaddw
%elif regsize == 512
   %define instruct1 vpaddw
%else
   %error unknown regsize
%endif
%endif


; main testcode macro

%if tcase == 1   ; Test case 1: same register

   %macro testcode 0
   %rep 100

      %ifnidni instruct1,none
         %if regsize < 256
            instruct1 reg0, reg0
         %else
            instruct1 reg0, reg0, reg0
         %endif
      %endif

      %rep  nummov
         mov1 reg0, reg0
      %endrep

   %endrep
   %endmacro

%elif tcase == 2   ; Test case 2: different register

   %macro testcode 0
   %if nummov == 0

      %rep 100
         %ifnidni instruct1,none
            %if regsize < 256
               instruct1 reg0, reg2
            %else
               instruct1 reg0, reg0, reg2
            %endif
         %endif
         
      %endrep

   %else   ; nummov > 0

      %rep 50

         %ifnidni instruct1,none
            %if regsize < 256
               instruct1 reg0, reg2
            %else
               instruct1 reg0, reg0, reg2
            %endif
         %endif

         %if nummov == 1
            mov1 reg1, reg0
         %elif nummov == 2   
            mov1 reg3, reg0
            mov1 reg1, reg3
         %elif nummov == 3   
            mov1 reg3, reg0
            mov1 reg4, reg3
            mov1 reg1, reg4
         %elif nummov == 4
            mov1 reg3, reg0
            mov1 reg4, reg3
            mov1 reg5, reg4
            mov1 reg1, reg5
         %elif nummov == 5
            mov1 reg3, reg0
            mov1 reg4, reg3
            mov1 reg5, reg4
            mov1 reg6, reg5
            mov1 reg1, reg6
         %else
            mov1 reg3, reg0
            %rep (nummov - 2) / 2
               mov1 reg4, reg3
               mov1 reg3, reg4
            %endrep
            mov1 reg1, reg3
         %endif  

         %ifnidni instruct1,none
            %if regsize < 256
               instruct1 reg1, reg2
            %else
               instruct1 reg1, reg1, reg2
            %endif
         %endif

         %if nummov == 1
            mov1 reg0, reg1
         %elif nummov == 2   
            mov1 reg3, reg1
            mov1 reg0, reg3
         %elif nummov == 3   
            mov1 reg3, reg1
            mov1 reg4, reg3
            mov1 reg0, reg4
         %elif nummov == 4
            mov1 reg3, reg1
            mov1 reg4, reg3
            mov1 reg5, reg4
            mov1 reg0, reg5
         %elif nummov == 5
            mov1 reg3, reg1
            mov1 reg4, reg3
            mov1 reg5, reg4
            mov1 reg6, reg5
            mov1 reg0, reg6
         %else
            mov1 reg3, reg1
            %rep (nummov - 2) / 2
               mov1 reg4, reg3
               mov1 reg3, reg4
            %endrep
            mov1 reg0, reg3
         %endif  

      %endrep
      
   %endif  ; nummov
   %endmacro

%elif tcase == 3   ; Test case 3: different register, known to be zero

   %macro testinit2 0
      ; set reg0 to zero
      %if regsize <= 64
         xor eax,eax
      %elif regsize == 65    ; 64 bit mmx registers
         pxor reg0,reg0
      %elif regsize == 128
         pxor reg0,reg0
      %elif regsize == 256
         vxorps reg0,reg0,reg0
      %elif regsize == 512
         vxorps reg0,reg0,reg0
      %endif
   %endmacro

   %macro testcode 0
   %rep 50
      mov1 reg1, reg0
      mov1 reg0, reg1
   %endrep
   %endmacro

%elif tcase == 4   ; Test case 4: move with zero extension

   %macro testcode 0
   %rep 100
      imul ax,ax,2134
      movzx eax,ax
   %endrep
   %endmacro

%else
   %error unknown test case tcase
%endif


; test loops
;%define repeat1 1000  ; defined in mov_rename.sh2
%define repeat2 1
