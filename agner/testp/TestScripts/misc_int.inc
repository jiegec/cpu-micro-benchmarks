; miscellaneous.inc                                              2017-04-05 Agner Fog
; Define test code for miscellaneous integer and general purpose instructions
; (c) Copyright 2013 by Agner Fog. GNU General Public License www.gnu.org/licenses

; instruction-specific test codes

; define NOP of variable length
%macro noplengthx 1
   %if %1 == 0
      ; nothing
   %elif %1 == 1
      nop
   %elif %1 == 2
      db 66H, 90H
   %elif %1 == 3
      db 0FH, 1FH, 0C0H
   %elif %1 == 4
      db 0FH, 1FH, 40H, 00H
   %elif %1 == 5
      db 0FH, 1FH, 44H, 00H, 00H
   %elif %1 == 6
      db 66H, 0FH, 1FH, 44H, 00H, 00H
   %elif %1 == 7
      db 0FH, 1FH, 80H, 00H, 00H, 00H, 00H
   %elif %1 == 8
      db 0FH, 1FH, 84H, 00H, 00H, 00H, 00H, 00H
   %elif %1 == 9
      db 66H, 0FH, 1FH, 84H, 00H, 00H, 00H, 00H, 00H
   %elif %1 == 10
      db 66H, 66H, 0FH, 1FH, 84H, 00H, 00H, 00H, 00H, 00H
   %elif %1 == 11
      db 66H, 66H, 66H, 0FH, 1FH, 84H, 00H, 00H, 00H, 00H, 00H
   %else
      %error "unknown noplength"
   %endif
%endmacro

; define optional lock prefix
%ifdef lockprefix
   %if lockprefix == 1
      %define prefix lock
   %else
      %define prefix
   %endif
%endif

; Define specific test code for each instruction case:

%ifidni instruct, call_without_return

   %if regsize == 32 
      %macro testinit2 0
         mov ebx,esp
      %endmacro
      %macro testafter1 0
         mov esp,ebx
      %endmacro
   %else
      %macro testinit2 0
         mov rbx,rsp
      %endmacro
      %macro testafter1 0
         mov rsp,rbx
      %endmacro
   %endif

   %macro testcode 0
      call $ + 8
      nop
      nop
      nop
   %endmacro

%elifidni instruct, call_and_return

   %ifndef nnops
      %define nnops 0
   %endif
   %macro testinit1 0
      jmp testinit1end
      nop
      align 32
      testfunction:  ; make a dummy function that we can call
      %rep nnops / 2
         nop
      %endrep
      ret
      align 16
      testinit1end:
   %endmacro
   %macro testcode 0
      call testfunction
      %rep nnops / 2
         nop
      %endrep
   %endmacro

%elifidni instruct, call_return_longnop

   %macro testinit1 0
      jmp testinit1end
      nop
      align 32
      testfunction:  ; make a dummy function that we can call
      ret
      align 16
      testinit1end:
   %endmacro
   %macro testcode 0
      call testfunction
      noplengthx noplen
   %endmacro

%elifidni instruct, push_call_return_imm

   %macro testinit1 0
      jmp testinit1end
      nop
      align 32
      testfunction:  ; make a dummy function that we can call
      ret regsize / 8
      align 16
      testinit1end:
   %endmacro
   %macro testcode 0
      push reg0
      call testfunction
      noplengthx noplen
   %endmacro

%elifidni instruct, call_register_and_return

   %macro testinit1 0
      jmp testinit1end
      nop
      align 32
      testfunction:  ; make a dummy function that we can call
      ret
      align 16
      testinit1end:
      %if regsize == 32 
         mov edi, testfunction
      %else
         lea rdi, [testfunction]
      %endif
   %endmacro
   %macro testcode 0
      call reg4
      noplengthx noplen
   %endmacro

%elifidni instruct, call_memory_and_return

   %macro testinit1 0
      jmp testinit1end
      nop
      align 32
      testfunction:  ; make a dummy function that we can call
      ret
      align 16
      testinit1end:
      %if regsize == 32 
         mov edi, testfunction
         mov [esi], edi
      %else
         lea rdi, [testfunction]
         mov [rsi], rdi
      %endif
   %endmacro
   %macro testcode 0
      call [reg5]
      noplengthx noplen
   %endmacro

%elifidni instruct, jmp
   %ifndef jmp_per_16b
      %define jmp_per_16b 2
   %endif

   %assign repcnt 0
   %macro testcode 0
      %if jmp_per_16b == 8
         jmp $+2
      %elif jmp_per_16b == 6
         %if repcnt % 3 == 0
            jmp $+2
         %else
            jmp $+3
            nop
         %endif
      %elif jmp_per_16b == 5
         %if repcnt % 5 == 0
            jmp $+4
            noplengthx 2
         %else
            jmp $+3
            nop
         %endif
      %elif jmp_per_16b == 4
         jmp $+4
         noplengthx 2
      %elif jmp_per_16b == 3
         %if repcnt % 3 == 0
            jmp $+6
            noplengthx 4
         %else
            jmp $+5
            noplengthx 3
         %endif
      %elif jmp_per_16b == 2
         jmp $+8
         noplengthx 6
      %elif jmp_per_16b == 1
         jmp $+16
         noplengthx 7
         noplengthx 7
      %else
         %error unknown jmp_per_16b
      %endif
      %assign repcnt repcnt+1
   %endmacro

%elifidni instruct, jmp_register
   %ifndef jmp_per_16b
      %define jmp_per_16b 1
   %endif
   %macro testcode 0
      %if jmp_per_16b == 1
         lea rbx, [rel $+16]
         jmp rbx
         noplengthx 7
      %else
         %error unknown jmp_per_16b
      %endif
   %endmacro

%elifidni instruct, jmp_memory
   %ifndef jmp_per_16b
      %define jmp_per_16b 1
   %endif
   %macro testcode 0
      %if jmp_per_16b == 1
         lea rbx, [rel $+16]
         mov [rsi],rbx
         jmp [rsi]
         noplengthx 4
      %else
         %error unknown jmp_per_16b
      %endif
   %endmacro

%elifidni instruct, conditional_jmp

   %ifndef jmp_per_16b
      %define jmp_per_16b 2
   %endif
   %ifndef jmptaken
      %define jmptaken true  ; can be true, false, alternate
   %endif

   %macro testinit3 0        ; set carry flag before repeat macro
      %ifidni jmptaken, no
         clc                 ; carry flag = 0
      %elifidni jmptaken, yes
         stc                 ; carry flag = 1
      %elifidni jmptaken, alternate
         %if regsize == 64
            mov eax, r14d    ; get carry flag from loop counter
         %else
            mov eax, dword [esp+8]
         %endif
         shr eax, 1
      %endif
   %endmacro

   %assign repcnt 0
   %macro testcode 0
      %if jmp_per_16b == 8
         jc $+2
      %elif jmp_per_16b == 6
         %if repcnt % 3 == 0
            jc $+2
         %else
            jc $+3
            nop
         %endif
      %elif jmp_per_16b == 5
         %if repcnt % 5 == 0
            jc $+4
            noplengthx 2
         %else
            jc $+3
            nop
         %endif
      %elif jmp_per_16b == 4
         jc $+4
         noplengthx 2
      %elif jmp_per_16b == 3
         %if repcnt % 3 == 0
            jc $+6
            noplengthx 4
         %else
            jc $+5
            noplengthx 3
         %endif
      %elif jmp_per_16b == 2
         jc $+8
         noplengthx 6
      %elif jmp_per_16b == 1
         jc $+16
         noplengthx 7
         noplengthx 7
      %else
         %error unknown jmp_per_16b
      %endif
      %assign repcnt repcnt+1
   %endmacro

%elifidni instruct, jecxz
   ; alternate, 2 per 16 bytes

   %macro testinit2 0        ; set ecx to bit 1 of loop counter
      %if modesize == 64
         mov ecx, r14d
      %else
         mov ecx, dword [esp+8]
      %endif
      and ecx, 2
   %endmacro
   %macro testcode 0
      jecxz $+8
      %if modesize == 64
         noplengthx 5  ; compensate for 67h prefix
      %else
         noplengthx 6
      %endif
   %endmacro

%elifidni instruct, jrcxz
   ; alternate, 2 per 16 bytes

   %macro testinit2 0        ; set ecx to bit 1 of loop counter
      mov ecx, r14d
      and ecx, 2
   %endmacro
   %macro testcode 0
      jrcxz $+8
      noplengthx 6
   %endmacro

%elifidni instruct, loop     ; 64 bit mode, 2 per 16 bytes

   %macro testinit2 0 
      %ifidni taken, alternate
         mov ebx, r14d       ; loop counter
         shr ebx, 1
         and ebx, 1
         inc ebx
      %else 
         mov ebx,10
      %endif
   %endmacro
   %macro testcode 0
      %ifidni taken, alternate
         mov ecx,ebx
         loop $+6
         noplengthx 4
      %else      ; taken = yes
         mov ecx,ebx
         loop $
         noplengthx 4
      %endif
   %endmacro

%elifidni instruct, loope     ; 64 bit mode, alternate, 2 per 16 bytes

   %macro testinit2 0 
      mov ebx, r14d       ; loop counter
      shr ebx, 1
      and ebx, 1
      inc ebx
   %endmacro
   %macro testinit3 0 
      cmp eax,eax
   %endmacro
   %macro testcode 0
      mov ecx,ebx
      loope $+6
      noplengthx 4
   %endmacro

%elifidni instruct, loopne     ; 64 bit mode, alternate, 2 per 16 bytes

   %macro testinit2 0 
      mov ebx, r14d       ; loop counter
      shr ebx, 1
      and ebx, 1
      inc ebx
   %endmacro
   %macro testcode 0
      mov ecx,ebx
      loopne $+6
      noplengthx 4
   %endmacro

%elifidni instruct, fused_alu_jmp     ; Possibly fused ALU instruction + conditional jump, 64 bit mode, alternate, 2 per 16 bytes
   ; instruct1 = cmp, test, add, and

   %macro testinit2 0 
      mov ebx, r14d       ; loop counter
      and ebx, 1
   %endmacro
   %macro testcode 0
      %ifidni instruct1, cmp
         cmp ebx,1
         jb  $+5
         noplengthx 3
      %elifidni instruct1, test
         test ebx,ebx
         jnz  $+6
         noplengthx 4
      %elifidni instruct1, add
         add ebx,0
         jnz  $+5
         noplengthx 3
      %elifidni instruct1, and
         and ebx,-1
         jz  $+5
         noplengthx 3
      %else
         %error unknown instruct1
      %endif
   %endmacro

%elifidni instruct, xlatb

   %macro testinit1 0 
      mov dword [rsi], 1
   %endmacro
   %macro testinit2 0 
      mov rbx, rsi
      xor eax,eax
   %endmacro
   %macro testcode 0
      %ifidni tmode, L
         xlatb
      %elifidni tmode, T
         xor eax,eax
         xlatb
      %endif
   %endmacro

%elifidni instruct, rdpmc

   %macro testinit2 0 
      xor ecx,ecx
   %endmacro

%elifidni instruct, inc

   %macro testcode 0
      %ifidni tmode, L  ; test latency of flag output
         inc eax
         sbb eax,eax
      %elifidni tmode, T
         inc eax
         sub eax,ebx
      %endif
   %endmacro

%elifidni instruct, lea

   ; parameters:
   ; tmode:    L = latency, T = throughput, R = throughput rip-relative, LM = latency to mov instruction, LA = latency to add instruction, 
   ; addrsize: 16, 32, 64  ; base and index register
   ; regsize:  16, 32, 64  ; destination register
   ; basereg:  0, 1
   ; scalef:   0, 1, 2, 4, 8
   ; ioffset:  0, 1, 4   bytes
   ; aprefix:  0, 1        ; address size prefix, overriding specified addrsize

   %define repeat1 100
   
   %ifndef aprefix
      %define aprefix 0
   %endif
   %ifndef tmode
      %define tmode L
   %endif
   %ifndef addrsize
      %define addrsize 64
   %endif
   %ifndef regsize
      %define regsize 32
   %endif
   %ifndef basereg
      %define basereg 1
   %endif
   %ifndef scalef
      %define scalef 0
   %endif
   %ifndef ioffset
      %define ioffset 1
   %endif

   %if basereg
      %if addrsize == 16
         %define basereg_  bx
      %elif addrsize == 32
         %define basereg_  ebx
      %elif addrsize == 64
         %define basereg_  rbx
      %endif
   %else
      %define basereg_
   %endif
   %if basereg
      %define plus1 +   ; + before scale*index
   %else
      %define plus1
   %endif
   %if basereg + scalef
      %define plus2 +   ; + before offset
   %else
      %define plus2
   %endif
   %if scalef
      %if addrsize == 16
         %define scaledindex plus1 si
      %elif addrsize == 32
         %define scaledindex  plus1 scalef*esi
      %elif addrsize == 64
         %define scaledindex  plus1 scalef*rsi
      %endif
   %else
      %define scaledindex
   %endif
   %if ioffset == 0
      %if basereg + scalef
         %define offset_
      %else
         %define offset_  0
      %endif
   %elif ioffset == 1
      %define offset_ plus2 5
   %else
      %define offset_ plus2 500
   %endif
   %if basereg
      %define destreg reg1
   %else
      %define destreg reg5
   %endif
   
   %macro testcode 0
      %if aprefix
         db 0x67   ; hard code address size prefix
      %endif 
      %ifidni tmode, L
         lea destreg , [ basereg_ scaledindex offset_ ]
      %elifidni tmode, T
         lea reg0 , [ basereg_ scaledindex offset_ ]
      %elifidni tmode, R
         lea reg2 , [ rel UserData ]
      %elifidni tmode, LM
         lea destreg , [ basereg_ scaledindex offset_ ]
         mov destreg, destreg
      %elifidni tmode, LA
         lea destreg , [ basereg_ scaledindex offset_ ]
         add destreg, destreg
      %endif
   %endmacro

%elifidni instruct, mov_r_m   ; mov register, memory, different addressing modes

   %macro testinit2 0
      xor ebx,ebx
   %endmacro
   %macro testcode 0
      %ifidni addrmode, INDIR             ; indirect addressing
         mov reg0, [rsi+rbx*4]
      %elifidni addrmode, RIP             ; rip relative addressing
         mov reg0, [rel UserData]
      %elifidni addrmode, ABS32           ; 32 bit absolute address
         mov reg0, [abs dword UserData]
      %elifidni addrmode, ABS64           ; 64 bit absolute address
         mov reg0, [abs qword UserData]
      %endif
   %endmacro

%elifidni instruct, mov_m_r   ; mov memory, register, different addressing modes

   %macro testinit2 0
      xor ebx,ebx
   %endmacro
   %macro testcode 0
      %ifidni addrmode, INDIR             ; indirect addressing
         mov [rsi+rbx*4], reg0
      %elifidni addrmode, RIP             ; rip relative addressing
         mov [rel UserData], reg0
      %elifidni addrmode, ABS32           ; 32 bit absolute address
         mov [abs dword UserData], reg0
      %elifidni addrmode, ABS64           ; 64 bit absolute address
         mov [abs qword UserData], reg0
      %endif
   %endmacro

%elifidni instruct, set

   %define repeat2 1
   %macro testcode 0
      %ifidni tmode, L         ; measure latency
         %rep 100
            sete al
            neg al
         %endrep
      %elifidni tmode, T         ; measure throughput with register operands
         %rep 50
            sete al
            setc bl
         %endrep
      %elifidni tmode, M         ; measure throughput with memory source operand
         %rep 50
            sete byte [rsi]
            setnc byte [rdi]
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

%elifidni instruct, vmaskmov  ; vmaskmovps / vmaskmovpd with memory source or destination operand
   ; specify instruct1 = vmaskmovps or vmaskmovpd
   ; specify tmode: TRM = throughput with memory source, LRM = latency with memory source,
   ;                TMR = throughput with memory destination, LMR = latency with memory destination
   ; specify immvalue to one of the values 0x00 0x02 0x55 0x33 0xff to define a mask

   %define repeat1 100
   %define repeat2 100

   %macro testinit2 0
      lea rsi, [UserData]
      ; make mask
      %ifidni instruct1, vmaskmovps
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
      %elifidni instruct1, vmaskmovpd
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
      %ifidni tmode, TRM           ; measure throughput with memory source
         instruct1 reg1,reg0,[rsi]
      %elifidni tmode, LRM         ; measure latency with memory source
         instruct1 reg1,reg0,[rsi]
         vmovaps [rsi], reg1
      %elifidni tmode, TMR           ; measure throughput with memory destination
         instruct1 [rsi],reg0,reg1
      %elifidni tmode, LMR           ; measure latency with memory destination
         instruct1 [rsi],reg0,reg1
         vmovaps reg1,[rsi]
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

%elifidni instruct, add

   %define repeat1 100
   %macro testcode 0
      prefix instruct [rsi],reg0
   %endmacro

%elifidni instruct, xadd

   %define repeat1 100
   %macro testcode 0
      prefix instruct [rsi],reg0
   %endmacro

%elifidni instruct, cmpxchg

   %define repeat1 100
   %macro testcode 0
      prefix instruct [rsi],reg0
   %endmacro

%elifidni instruct, cmpxchg8b

   %define repeat1 100
   %macro testcode 0
      prefix instruct [rsi]
   %endmacro

%elifidni instruct, cmpxchg16b

   %define repeat1 100
   %macro testcode 0
      prefix instruct [rsi]
   %endmacro


%else

   %error unknown instruct

;   %define repeat1 0       ; disable default loops
;   %define repeat2 1


%endif

