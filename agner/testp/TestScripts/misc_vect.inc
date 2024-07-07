; miscellaneous.inc                                              2013-07-06 Agner Fog
; Define test code for miscellaneous vector instructions
; (c) Copyright 2013 by Agner Fog. GNU General Public License www.gnu.org/licenses

; instruction-specific test codes

; Define specific test code for each instruction case:


%ifidni instruct, movdqu  ; unaligned read/write
   
   %define repeat1 100
   %define repeat2 1
   %macro testcode 0
      %ifidni tmode, L         ; measure latency
         %rep 100
            instruct1 reg0, sizeptr [rsi+moffset]
            instruct1 sizeptr [rsi+moffset], reg0
         %endrep
      %elifidni tmode, MR         ; measure throughput for unaligned read
         %rep 50
            instruct1 reg0, sizeptr [rsi+moffset]
            instruct1 reg1, sizeptr [rdi+moffset]
         %endrep
      %elifidni tmode, MW        ; measure throughput for unaligned write
         %rep 50
            instruct1 sizeptr [rsi+moffset], reg0
            instruct1 sizeptr [rdi+moffset], reg1
         %endrep
      %else
         %error unknown testmode
      %endif
   %endmacro


%elifidni instruct, pcmpestri

   %define repeat1 100
   %macro testcode 0
      %ifidni tmode, L
         pcmpestri xmm1,xmm1,immvalue
         movd xmm1,ecx
      %elifidni tmode, T
         pcmpestri xmm1,xmm2,immvalue
      %elifidni tmode, M
         pcmpestri xmm1,[rsi],immvalue
      %endif
   %endmacro

%elifidni instruct, pcmpestrm

   %define repeat1 100
   %macro testcode 0
      %ifidni tmode, L
         pcmpestrm xmm0,xmm0,immvalue
      %elifidni tmode, T
         pcmpestrm xmm1,xmm2,immvalue
      %elifidni tmode, M
         pcmpestrm xmm1,[rsi],immvalue
      %endif
   %endmacro

%elifidni instruct, pcmpistri

   %define repeat1 100
   %macro testcode 0
      %ifidni tmode, L
         pcmpistri xmm1,xmm1,immvalue
         movd xmm1,ecx
      %elifidni tmode, T
         pcmpistri xmm1,xmm2,immvalue
      %elifidni tmode, M
         pcmpistri xmm1,[rsi],immvalue
      %endif
   %endmacro

%elifidni instruct, pcmpistrm

   %define repeat1 100
   %macro testcode 0
      %ifidni tmode, L
         pcmpistrm xmm0,xmm0,immvalue
      %elifidni tmode, T
         pcmpistrm xmm1,xmm2,immvalue
      %elifidni tmode, M
         pcmpistrm xmm1,[rsi],immvalue
      %endif
   %endmacro

%elifidni instruct, insertps

   %define repeat1 100
   %define repeat2 1
   %macro testcode 0
      %ifidni tmode, L
         %rep 100
            instruct xmm0,xmm0,immvalue
         %endrep
      %elifidni tmode, T
         %rep 25
            instruct xmm0,xmm1,immvalue
            instruct xmm2,xmm3,immvalue
            instruct xmm4,xmm5,immvalue
            instruct xmm6,xmm7,immvalue
         %endrep
      %elifidni tmode, M
         %rep 100
            instruct xmm0,[rsi],immvalue
         %endrep
      %elifidni tmode, LM
         %rep 100
            instruct xmm0,[rsi],immvalue
            movss dword [rsi],xmm0
         %endrep
      %endif
   %endmacro

%elifidni instruct, vinsertps

   %define repeat1 100
   %macro testcode 0
      %ifidni tmode, L
         instruct reg0,reg0,reg0,immvalue
      %elifidni tmode, T
         instruct reg0,reg1,reg2,immvalue
      %elifidni tmode, M
         instruct reg0,reg1,[rsi],immvalue
      %elifidni tmode, LM
         instruct reg0,reg1,[rsi],immvalue
         vmovss dword [rsi],xmm0
      %endif
   %endmacro

%elifidni instruct, extractps

   %define repeat1 100
   %macro testcode 0
      %ifidni tmode, L
         instruct eax,reg0,immvalue
         movd reg0,eax
      %elifidni tmode, T
         instruct eax,reg0,immvalue
      %elifidni tmode, M
         instruct [rsi],reg0,immvalue
      %elifidni tmode, LM
         instruct [rsi],reg0,immvalue
         movss xmm0,[rsi]
      %endif
   %endmacro

%elifidni instruct, vinsertf128

   %define repeat1 100
   %macro testcode 0
      %ifidni tmode, L
         instruct ymm0,ymm0,xmm0,immvalue
      %elifidni tmode, T
         instruct ymm0,ymm1,xmm2,immvalue
      %elifidni tmode, M
         instruct ymm0,ymm1,[rsi],immvalue
      %elifidni tmode, LM
         instruct ymm0,ymm1,[rsi],immvalue
         vmovaps [rsi],ymm0
      %endif
   %endmacro

%elifidni instruct, vextractf128

   %define repeat1 100
   %macro testcode 0
      %ifidni tmode, L
         instruct xmm0,ymm0,immvalue
      %elifidni tmode, T
         instruct xmm0,ymm1,immvalue
      %elifidni tmode, M
         instruct [rsi],ymm1,immvalue
      %elifidni tmode, LM
         instruct [rsi],ymm1,immvalue
         vmovaps xmm1,[rsi]
      %endif
   %endmacro

%elifidni instruct, vbroadcastss

   %define repeat1 100
   %macro testcode 0
      %ifidni tmode, L
         instruct reg0,dword [rsi]
         vmovss [rsi], xmm0
      %elifidni tmode, T
         instruct reg0,dword [rsi]
      %endif
   %endmacro

%elifidni instruct, vbroadcastsd

   %define repeat1 100
   %macro testcode 0
      %ifidni tmode, L
         instruct reg0, [rsi]
         vmovsd [rsi], xmm0
      %elifidni tmode, T
         instruct reg0, [rsi]
      %endif
   %endmacro

%elifidni instruct, vbroadcastf128

   %define repeat1 100
   %macro testcode 0
      %ifidni tmode, L
         instruct ymm0, [rsi]
         vmovaps [rsi], xmm0
      %elifidni tmode, T
         instruct ymm0, [rsi]
      %endif
   %endmacro

%elifidni instruct, blendvps

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

%elifidni instruct, maskmovq

   %define repeat1 100
   %macro testinit2 0
      %if   immvalue == 0x00     ; all bytes 00
         pxor mm0,mm0      
      %elif immvalue == 0x02     ; one byte ff
         mov eax, 0000ff00h
         movd  mm0, eax
      %elif immvalue == 0x55     ; alternate 00 ff bytes
         pcmpeqw mm0,mm0
         psrlw   mm0, 8      
      %elif immvalue == 0x33     ; alternate 00 00 ff ff bytes
         pcmpeqw mm0,mm0
         psrld   mm0, 16     
      %elif immvalue == 0xFF     ; all bytes ff
         pcmpeqw mm0,mm0     
      %else
         %error unsupported immvalue
      %endif
   %endmacro
   %macro testcode 0
      %ifidni tmode, T           ; measure throughput
         maskmovq mm1, mm0
      %elifidni tmode, L         ; measure latency
         maskmovq mm1, mm0
         movq mm1, [rdi]
      %else
         %error unknown testmode
      %endif
   %endmacro

%elifidni instruct, maskmovdqu

   %define repeat1 100
   %macro testinit2 0
      %if immvalue   == 0x00     ; all bytes 00
         pxor xmm0,xmm0   
      %elif immvalue == 0x02     ; one byte ff
         mov eax, 0000ff00h
         movd  xmm0, eax
      %elif immvalue == 0x55     ; alternate 00 ff bytes
         pcmpeqw xmm0,xmm0
         psrlw   xmm0, 8    
      %elif immvalue == 0x33     ; alternate 00 00 ff ff bytes
         pcmpeqw xmm0,xmm0
         psrld   xmm0, 16  
      %elif immvalue == 0xFF     ; all bytes ff
         pcmpeqw xmm0,xmm0   
      %else
         %error unsupported immvalue
      %endif
   %endmacro
   %macro testcode 0
      %ifidni tmode, T           ; measure throughput
         maskmovdqu xmm1, xmm0
      %elifidni tmode, L         ; measure latency
         maskmovdqu xmm1, xmm0
         movdqa xmm1, [rdi]
      %else
         %error unknown testmode
      %endif
   %endmacro

%elifidni instruct, vmaskmov  ; vmaskmovps, vmaskmovpd, vpmaskmovd, vpmaskmovq 
   ;                            with memory source or destination operand
   ; specify instruct1 = vmaskmovps, vmaskmovpd, vpmaskmovd, vpmaskmovq 
   ; specify tmode: TRM = throughput with memory source, LRM = latency with memory source,
   ;                TMR = throughput with memory destination, LMR = latency with memory destination
   ; specify immvalue to one of the values 0x00 0x02 0x55 0x33 0xff to define a mask

   %define repeat1 100
   %define repeat2 100

   %macro testinit2 0
      lea rsi, [UserData]
      ; determine block size and move instruction
      %ifidni instruct1, vmaskmovps
         %define blocksize 4
         %define moveinstr vmovaps
      %elifidni instruct1, vmaskmovpd
         %define blocksize 8
         %define moveinstr vmovaps
      %elifidni instruct1, vpmaskmovd
         %define blocksize 4
         %define moveinstr vmovdqa
      %elifidni instruct1, vpmaskmovq
         %define blocksize 8
         %define moveinstr vmovdqa
      %else
         %error unknown instruction instruct1
      %endif      
      
      ; make mask
      %if blocksize == 4
         %if immvalue   == 0x00     ; all bytes 00
            vpxor xmm0,xmm0,xmm0   
         %elif immvalue == 0x02     ; one dword ff
            mov eax, -1
            vmovd  xmm0, eax
            vshufps ymm0,ymm0,ymm0,04h
         %elif immvalue == 0x55     ; alternate 00 ff dwords
            mov eax, -1
            vmovd  xmm0, eax
            vshufps ymm0,ymm0,ymm0,11h
            vinsertf128 ymm0,ymm0,xmm0,1
         %elif immvalue == 0x33     ; alternate 00 00 ff ff bytes
            mov eax, -1
            vmovd  xmm0, eax
            vshufps ymm0,ymm0,ymm0,05h
            vinsertf128 ymm0,ymm0,xmm0,1
         %elif immvalue == 0xFF     ; all bytes ff
            vpcmpeqw xmm0,xmm0,xmm0
            vinsertf128 ymm0,ymm0,xmm0,1
         %else
            %error unsupported immvalue
         %endif
      %else  ; blocksize == 8
         %if immvalue   == 0x00     ; all bytes 00
            vpxor xmm0,xmm0,xmm0   
         %elif immvalue == 0x02     ; one qword ff
            mov rax, -1
            vmovq  xmm0, rax
            vshufpd xmm0,xmm0,xmm0,02h
         %elif immvalue == 0x55     ; alternate 00 ff qwords
            mov rax, -1
            vmovq  xmm0, rax
            vshufpd ymm0,ymm0,ymm0,05h
            vinsertf128 ymm0,ymm0,xmm0,1
         %elif immvalue == 0x33     ; alternate 00 00 ff ff qwords
            vpcmpeqw xmm0,xmm0,xmm0
         %elif immvalue == 0xFF     ; all bytes ff
            vpcmpeqw xmm0,xmm0,xmm0
            vinsertf128 ymm0,ymm0,xmm0,1
         %else
            %error unsupported immvalue
         %endif
      %endif
   %endmacro
   %macro testcode 0
      %ifidni tmode, TRM             ; measure throughput with memory source
         instruct1 reg1,reg0,[rsi]
      %elifidni tmode, LRM           ; measure latency with memory source
         instruct1 reg1,reg0,[rsi]
         moveinstr [rsi], reg1
      %elifidni tmode, TMR           ; measure throughput with memory destination
         instruct1 [rsi],reg0,reg1
      %elifidni tmode, LMR           ; measure latency with memory destination
         instruct1 [rsi],reg0,reg1
         moveinstr reg1,[rsi]
      %else
         %error unknown testmode
      %endif
   %endmacro

%elifidni instruct, ldmxcsr

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

%elifidni instruct, stmxcsr

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

%elifidni instruct, vstmxcsr

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

%elifidni instruct, xgetbv

   %define repeat1 100
   %define repeat2 100
   %macro testinit2 0
      xor ecx, ecx
   %endmacro

%elifidni instruct, ptest

   %define repeat1 100
   %macro testcode 0
      %ifidni tmode, T         ; measure throughput with register operands
         instruct1 reg0,reg1
      %elifidni tmode, L       ; measure latency
         instruct1 reg0,reg0
         sbb eax,eax
         ;vmovd xmm0,eax
         movd xmm0,eax
      %elifidni tmode, M
         instruct1 reg0,[rsi]
      %endif
   %endmacro
   
%elifidni instruct, vptest

   %define repeat1 100
   %macro testcode 0
      %ifidni tmode, T         ; measure throughput with register operands
         instruct1 reg0,reg1
      %elifidni tmode, L       ; measure latency
         instruct1 reg0,reg0
         sbb eax,eax
         vmovd xmm0,eax
         ;movd xmm0,eax
      %elifidni tmode, M
         instruct1 reg0,[rsi]
      %endif
   %endmacro
   
%elifidni instruct, pcmpestri

   %define repeat1 100
   %macro testcode 0
      %ifidni tmode, L
         pcmpestri xmm1,xmm1,immvalue
         movd xmm1,ecx
      %elifidni tmode, T
         pcmpestri xmm1,xmm2,immvalue
      %elifidni tmode, M
         pcmpestri xmm1,[rsi],immvalue
      %endif
   %endmacro

%elifidni instruct, pcmpestrm

   %define repeat1 100
   %macro testcode 0
      %ifidni tmode, L
         pcmpestrm xmm0,xmm0,immvalue
      %elifidni tmode, T
         pcmpestrm xmm1,xmm2,immvalue
      %elifidni tmode, M
         pcmpestrm xmm1,[rsi],immvalue
      %endif
   %endmacro

%elifidni instruct, pcmpistri

   %define repeat1 100
   %macro testcode 0
      %ifidni tmode, L
         pcmpistri xmm1,xmm1,immvalue
         movd xmm1,ecx
      %elifidni tmode, T
         pcmpistri xmm1,xmm2,immvalue
      %elifidni tmode, M
         pcmpistri xmm1,[rsi],immvalue
      %endif
   %endmacro

%elifidni instruct, pcmpistrm

   %define repeat1 100
   %macro testcode 0
      %ifidni tmode, L
         pcmpistrm xmm0,xmm0,immvalue
      %elifidni tmode, T
         pcmpistrm xmm1,xmm2,immvalue
      %elifidni tmode, M
         pcmpistrm xmm1,[rsi],immvalue
      %endif
   %endmacro

%elifidni instruct, insertps

   %define repeat1 100
   %macro testcode 0
      %ifidni tmode, L
         %rep 100
            instruct xmm0,xmm0,immvalue
         %endrep
      %elifidni tmode, T
         %rep 25
            instruct xmm0,xmm1,immvalue
            instruct xmm2,xmm3,immvalue
            instruct xmm4,xmm5,immvalue
            instruct xmm6,xmm7,immvalue
         %endrep
      %elifidni tmode, M
         %rep 100
            instruct xmm0,[rsi],immvalue
         %endrep
      %elifidni tmode, LM
         %rep 100
            instruct xmm0,[rsi],immvalue
            movss dword [rsi],xmm0
         %endrep
      %endif
   %endmacro

%elifidni instruct, vinsertps

   %define repeat1 100
   %macro testcode 0
      %ifidni tmode, L
         instruct reg0,reg0,reg0,immvalue
      %elifidni tmode, T
         instruct reg0,reg1,reg2,immvalue
      %elifidni tmode, M
         instruct reg0,reg1,[rsi],immvalue
      %elifidni tmode, LM
         instruct reg0,reg1,[rsi],immvalue
         vmovss dword [rsi],xmm0
      %endif
   %endmacro

%elifidni instruct, extractps

   %define repeat1 100
   %macro testcode 0
      %ifidni tmode, L
         instruct eax,reg0,immvalue
         movd reg0,eax
      %elifidni tmode, T
         instruct eax,reg0,immvalue
      %elifidni tmode, M
         instruct [rsi],reg0,immvalue
      %elifidni tmode, LM
         instruct [rsi],reg0,immvalue
         movss xmm0,[rsi]
      %endif
   %endmacro

%elifidni instruct, vinsertf128

   %define repeat1 100
   %macro testcode 0
      %ifidni tmode, L
         instruct ymm0,ymm0,xmm0,immvalue
      %elifidni tmode, T
         instruct ymm0,ymm1,xmm2,immvalue
      %elifidni tmode, M
         instruct ymm0,ymm1,[rsi],immvalue
      %elifidni tmode, LM
         instruct ymm0,ymm1,[rsi],immvalue
         vmovaps [rsi],ymm0
      %endif
   %endmacro

%elifidni instruct, vextractf128

   %define repeat1 100
   %macro testcode 0
      %ifidni tmode, L
         instruct xmm0,ymm0,immvalue
      %elifidni tmode, T
         instruct xmm0,ymm1,immvalue
      %elifidni tmode, M
         instruct [rsi],ymm1,immvalue
      %elifidni tmode, LM
         instruct [rsi],ymm1,immvalue
         vmovaps xmm1,[rsi]
      %endif
   %endmacro

%elifidni instruct, vbroadcastss

   %define repeat1 100
   %macro testcode 0
      %ifidni tmode, L
         instruct reg0,dword [rsi]
         vmovss [rsi], xmm0
      %elifidni tmode, T
         instruct reg0,dword [rsi]
      %endif
   %endmacro

%elifidni instruct, vbroadcastsd

   %define repeat1 100
   %macro testcode 0
      %ifidni tmode, L
         instruct reg0, [rsi]
         vmovsd [rsi], xmm0
      %elifidni tmode, T
         instruct reg0, [rsi]
      %endif
   %endmacro

%elifidni instruct, vbroadcastf128

   %define repeat1 100
   %macro testcode 0
      %ifidni tmode, L
         instruct ymm0, [rsi]
         vmovaps [rsi], xmm0
      %elifidni tmode, T
         instruct ymm0, [rsi]
      %endif
   %endmacro

%elifidni instruct, pblendvb

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

%elifidni instruct, crc32

   %define repeat1 100
   %define repeat2 1
   %macro testcode 0
      %ifidni tmode, L         ; measure latency
         %rep 100
            instruct eax, reg0
         %endrep
      %elifidni tmode, T         ; measure throughput with register operands
         %rep 25
            instruct eax, reg1
            instruct ecx, reg3
            instruct edi, reg5
            instruct ebp, reg7
         %endrep
      %elifidni tmode, M         ; measure throughput with memory source operand
         %rep 50
            instruct eax, sizeptr [rsi]
            instruct ebx, sizeptr [rdi]
         %endrep
      %else
         %error unknown testmode
      %endif
   %endmacro
   
%elifidni instruct, vshift  ; vpsllw ymm,ymm,xmm  , etc.
   
   %define repeat1 100
   %define repeat2 1
   %macro testcode 0
      %ifidni tmode, L         ; measure latency
         %rep 100
            instruct1 reg0, reg0, xmm5
         %endrep
      %elifidni tmode, T         ; measure throughput with register operands
         %rep 25
            instruct1 reg0, reg4, xmm5
            instruct1 reg1, reg4, xmm5
            instruct1 reg2, reg4, xmm5
            instruct1 reg3, reg4, xmm5
         %endrep
      %elifidni tmode, M         ; measure throughput with memory source operand
         %rep 50
            instruct1 reg0, reg4, oword [rsi]
            instruct1 reg1, reg4, oword [rdi]
         %endrep
      %else
         %error unknown testmode
      %endif
   %endmacro

%else

    %error unknown instruct

%endif


;   %define repeat1 0       ; disable default loops
;   %define repeat2 1

