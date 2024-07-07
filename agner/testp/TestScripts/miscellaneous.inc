; miscellaneous.inc
; Define test code for miscellaneous vector instructions
; Last modified 2020-08-16
; (c) Copyright 2013-2020 by Agner Fog. GNU General Public License www.gnu.org/licenses

; Parameters:
; instruct:  instruction to test
; modelins:  model instruction, if several instructions use the same model
; tmode:     test mode
; other parameters as specified in templateb64.nasm or under each model case


; Define specific test code for each instruction case or model instruction

%ifndef modelins
   %define modelins instruct
%endif


%ifidni modelins, movdqu  ; unaligned read/write
   
   %define repeat1 100
   %define repeat2 1
   %ifndef instruct2
      %define instruct2 instruct
   %endif

   %macro testcode 0
      %ifidni tmode, L            ; measure latency
         %rep 100
            instruct reg0, sizeptr [rsi+moffset]
            instruct2 sizeptr [rsi+moffset], reg0
         %endrep
      %elifidni tmode, TR         ; measure throughput for unaligned read
         %rep 50
            instruct reg0, sizeptr [rsi+moffset]
            instruct reg1, sizeptr [rdi+moffset]
         %endrep
      %elifidni tmode, TW         ; measure throughput for unaligned write
         %rep 50
            instruct sizeptr [rsi+moffset], reg0
            instruct sizeptr [rdi+moffset], reg1
         %endrep
      %else
         %error unknown testmode
      %endif
   %endmacro


%elifidni modelins, pcmpestri

   %define repeat1 100
   %macro testcode 0
      %ifidni tmode, L
         instruct xmm1,xmm1,immvalue
         movd xmm1,ecx
      %elifidni tmode, T
         instruct xmm1,xmm2,immvalue
      %elifidni tmode, M
         instruct xmm1,[rsi],immvalue
      %endif
   %endmacro

%elifidni modelins, pcmpestrm

   %define repeat1 100
   %macro testcode 0
      %ifidni tmode, L
         instruct xmm0,xmm0,immvalue
      %elifidni tmode, T
         instruct xmm1,xmm2,immvalue
      %elifidni tmode, M
         instruct xmm1,[rsi],immvalue
      %endif
   %endmacro

%elifidni modelins, pcmpistri

   %define repeat1 100
   %macro testcode 0
      %ifidni tmode, L
         instruct xmm1,xmm1,immvalue
         movd xmm1,ecx
      %elifidni tmode, T
         instruct xmm1,xmm2,immvalue
      %elifidni tmode, M
         instruct xmm1,[rsi],immvalue
      %endif
   %endmacro

%elifidni modelins, pcmpistrm

   %define repeat1 100
   %macro testcode 0
      %ifidni tmode, L
         instruct xmm0,xmm0,immvalue
      %elifidni tmode, T
         instruct xmm1,xmm2,immvalue
      %elifidni tmode, M
         instruct xmm1,[rsi],immvalue
      %endif
   %endmacro

%elifidni modelins, blendvps

   %define repeat1 100
   %define repeat2 1
   %macro testcode 0
      %ifidni tmode, L         ; measure latency
         %rep 100
            instruct reg0, reg0, xmm0
         %endrep
      %elifidni tmode, T         ; measure throughput with register operands
         %rep 50
            instruct reg1, reg2, xmm0
            instruct reg3, reg4, xmm0
         %endrep
      %elifidni tmode, M         ; measure throughput with memory source operand
         %rep 50
            instruct reg1, [rsi], xmm0
            instruct reg2, [rsi], xmm0
         %endrep
      %else
         %error unknown testmode
      %endif
   %endmacro

%elifidni modelins, pblendvb

   %define repeat1 100
   %define repeat2 1
   %macro testcode 0
      %ifidni tmode, L         ; measure latency
         %rep 100
            instruct reg0, reg1, xmm0
         %endrep
      %elifidni tmode, T         ; measure throughput with register operands
         %rep 50
            instruct reg1, reg2, xmm0
            instruct reg3, reg4, xmm0
         %endrep
      %elifidni tmode, M         ; measure throughput with memory source operand
         %rep 50
            instruct reg1, [rsi], xmm0
            instruct reg2, [rsi], xmm0
         %endrep
      %else
         %error unknown testmode
      %endif
   %endmacro

%elifidni modelins, maskmovq  ; maskmovq or maskmovdqu
   ;                            with memory source or destination operand
   ; Parameters:
   ; instruct = maskmovq or maskmovdqu
   ; tmode: T = throughput with memory destination, 
   ;        L = latency with memory destination
   ; immvalue = bits to define a mask

   %define repeat1 100

   ; make mask with byte granularity
   %macro testdata 0
      %assign i 0
      %rep 32
         DB  -((immvalue >> i) & 1)
         %assign i i+1
      %endrep
      times 1000H  DB 0
   %endmacro

   ; load mask
   %macro testinit1 0
      %ifidni regtype, mmx
         movq mm0, [psi]
      %elif regsize == 128
         movdqa xmm0, [psi]
      %elif regsize == 256
         vmovdqa ymm0, [psi]
      %else
         vmovdqa32 reg0, [psi]
      %endif
   %endmacro

   %macro testcode 0
      %ifidni tmode, T           ; measure throughput
         instruct reg1, reg0
      %elifidni tmode, L         ; measure latency
         instruct reg1, reg0
         %ifidni regtype, mmx
            movq mm1, [rdi]
         %elif regsize == 128
            movdqa xmm1, [rdi]
         %elif regsize == 256
            vmovdqa ymm1, [rdi]
         %else
            vmovdqa32 reg1, [rdi]
         %endif
      %else
         %error unknown testmode
      %endif
   %endmacro

%elifidni modelins, vmaskmov  ; vmaskmovps, vmaskmovpd, vpmaskmovd, vpmaskmovq 
   ;                            with memory source or destination operand
   ; Parameters:
   ; instruct = vmaskmovps, vmaskmovpd, vpmaskmovd, vpmaskmovq 
   ; tmode: TR = throughput with memory source,
   ;        LR = latency with memory source,
   ;        TW = throughput with memory destination,
   ;        LW = latency with memory destination
   ; elementsize = 32 or 64 bits
   ; immvalue = bits to define a mask

   %define repeat1 100

   ; make mask with 32 or 64 bit granularity
   %macro testdata 0
      %assign i 0
      %rep 32
         %if elementsize == 8
            DB  -((immvalue >> i) & 1)
         %elif elementsize == 16
            DW  -((immvalue >> i) & 1)
         %elif elementsize == 32
            DD  -((immvalue >> i) & 1)
         %else ; 64
            DQ  -((immvalue >> i) & 1)
         %endif
         %assign i i+1
      %endrep
      times 1000H  DB 0
   %endmacro

   ; load mask
   %macro testinit1 0
      %if regsize <= 256
         vmovdqa reg0, [psi]
      %else
         vmovdqa32 reg0, [psi]
      %endif
   %endmacro

   %macro testcode 0
      %ifidni tmode, TR          ; measure throughput with memory source
         instruct reg1, reg0, [rdi]
      %elifidni tmode, LR        ; measure latency with memory source
         instruct reg1, reg0, [rdi]
         %if regsize <= 256
            vmovdqa [rdi], reg1
         %else
            vmovdqa32 [rdi], reg1
         %endif
      %elifidni tmode, TW         ; measure throughput with memory destination
         instruct [rdi], reg0, reg1
      %elifidni tmode, LW         ; measure latency with memory destination
         instruct [rdi], reg0, reg1
         %if regsize <= 256
            vmovdqa reg1, [rdi]
         %else
            vmovdqa32 reg1, [rdi]
         %endif
      %else
         %error unknown testmode
      %endif
   %endmacro

%elifidni modelins, vmovdqu8       ; vmovdqa32/64, vmovdqu8/16/32/64 
   ;                                 with memory source or destination operand and mask
   ; Parameters:
   ; instruct = vmovdqa32/64, vmovdqu8/16/32/64 
   ; tmode: TR = throughput with memory source,
   ;        LR = latency with memory source,
   ;        TW = throughput with memory destination,
   ;        LW = latency with memory destination
   ; elementsize = 8, 16, 32 or 64 bits
   ; immvalue = bits to define a mask
   ; usezero  = 1 for zeroing option {z}
   ; moffset  = memory offset if unaligned

   %define repeat1 100
   %ifndef moffset
      %define moffset 0
   %endif

   ; load mask
   %macro testinit1 0
      %assign numelements regsize/elementsize
      %if numelements > 32
         mov rax, immvalue
         kmovq k1, rax
      %elif numelements > 16
         mov eax, immvalue
         kmovd k1, eax
      %else
         mov eax, immvalue
         kmovw k1, eax
      %endif
   %endmacro

   %macro testcode 0

      ; zeroing option
      %ifndef usezero
         %define usezero 0
      %endif
      %if usezero == 0
         %define zop
      %else
         %define zop {z}
      %endif

      %ifidni tmode, TR          ; measure throughput with memory source
         instruct reg1{k1} zop, [rsi+moffset]
      %elifidni tmode, LR        ; measure latency with memory source
         instruct reg1{k1} zop, [rsi+moffset]
         %if regsize <= 256
            vmovdqa [rsi+moffset], reg1
         %else
            vmovdqa32 [rsi+moffset], reg1
         %endif
      %elifidni tmode, TW         ; measure throughput with memory destination
         instruct [rsi+moffset]{k1} zop, reg1
      %elifidni tmode, LW         ; measure latency with memory destination
         instruct [rsi+moffset]{k1} zop, reg1
         %if regsize <= 256
            vmovdqa reg1, [rsi+moffset]
         %else
            vmovdqa32 reg1, [rsi+moffset]
         %endif
      %else
         %error unknown testmode
      %endif
   %endmacro


%elifidni modelins, vpsllw

   %define repeat1 100
   %define repeat2 1
   %macro testinit2 0
      mov  eax, 5
      vmovd xmm0, eax
   %endmacro
   %macro testcode 0
      %ifidni tmode, L           ; measure latency
         %rep 100
            instruct reg1, reg1, xmm0
         %endrep
      %endif
      %ifidni tmode, T           ; measure throughput
         %rep 100
            instruct reg2, reg1, xmm0
         %endrep
      %endif
   %endmacro

%elifidni modelins, ldmxcsr

   %define repeat1 100
   %define repeat2 1
   %macro testinit2 0
      stmxcsr [rsi+16]
      mov  eax, [rsi+16]
      xor  eax, 8040h
      mov  [rsi], eax
   %endmacro
   %macro testcode 0
      %ifidni tmode, T           ; measure throughput
         %rep 50
            ldmxcsr [rsi]
            ldmxcsr [rsi+16]     ; alternate between different values
         %endrep
      %endif
   %endmacro

%elifidni modelins, stmxcsr

   %define repeat1 100
   %define repeat2 100
   %macro testinit2 0
      stmxcsr [rsi]
   %endmacro
   %macro testcode 0
      %ifidni tmode, T           ; measure throughput
         stmxcsr [rsi]
      %elifidni tmode, L         ; measure latency + ldmxcsr
         ldmxcsr [rsi]
         stmxcsr [rsi]
      %endif
   %endmacro

%elifidni modelins, vstmxcsr

   %define repeat1 100
   %define repeat2 100
   %macro testinit2 0
      vstmxcsr [rsi]
   %endmacro
   %macro testcode 0
      %ifidni tmode, T           ; measure throughput
         vstmxcsr [rsi]
      %elifidni tmode, L         ; measure latency + ldmxcsr
         vldmxcsr [rsi]
         vstmxcsr [rsi]
      %endif
   %endmacro

%elifidni modelins, xgetbv

   %define repeat1 100
   %define repeat2 100
   %macro testinit2 0
      xor ecx, ecx
   %endmacro
   %macro testcode 0
      xgetbv
   %endmacro
  
%elifidni modelins, crc32

   %define repeat1 100
   %define repeat2 1
   %macro testcode 0
      %ifidni tmode, L           ; measure latency
         %rep 100
            %if regsize < 64
               instruct eax, reg0
            %else
               instruct rax, reg0
            %endif
         %endrep
      %elifidni tmode, T         ; measure throughput with register operands
         %rep 25
            %if regsize < 64
               instruct eax, reg1
               instruct ecx, reg3
               instruct edi, reg5
               instruct ebp, reg7
            %else
               instruct rax, reg1
               instruct rcx, reg3
               instruct rdi, reg5
               instruct r8,  reg7
            %endif
         %endrep
      %elifidni tmode, M         ; measure throughput with memory source operand
         %rep 50
            %if regsize < 64
               instruct eax, sizeptr [rsi]
               instruct ebx, sizeptr [rdi]
            %else
               instruct rax, sizeptr [rsi]
               instruct rbx, sizeptr [rdi]
            %endif
         %endrep
      %else
         %error unknown testmode
      %endif
   %endmacro
   

%elifidni modelins, fxsave

   %define repeat1 100
   %define repeat2 1
   %macro testinit1 0
      modelins [psi]                ; = esi or rsi
   %endmacro
   %macro testinit2 0
      or eax,-1
      cdq
   %endmacro

   %macro testcode 0
      %ifidni tmode, T           ; measure throughput
         %rep 100
            instruct [psi]
         %endrep
      %elifidni tmode, L         ; measure latency fxsave + fxrstor
         %rep 100
            instruct [psi]
            instruct2 [psi]
         %endrep
      %else
         %error unknown testmode
      %endif
   %endmacro

%elifidni modelins, xsave

   %define repeat1 100
   %define repeat2 1
   %macro testinit1 0
      modelins [psi]                ; = esi or rsi
   %endmacro
   %macro testinit2 0
      modelins [psi]                ; = esi or rsi
   %endmacro

   %macro testcode 0
      %ifidni tmode, T           ; measure throughput
         %rep 100
            instruct [psi]
         %endrep
      %elifidni tmode, L         ; measure latency xsave + xrstor
         %rep 100
            instruct [psi]
            instruct2 [psi]
         %endrep
      %else
         %error unknown testmode
      %endif
   %endmacro

%else

    %error unknown model instruction modelins

%endif


;   %define repeat1 0       ; disable default loops
;   %define repeat2 1

