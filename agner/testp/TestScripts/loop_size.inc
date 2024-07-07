;----------------------------------------------------------------------------
;        loop_size.inc                          2021-01-29 Agner Fog
;
; PMC Test program for testing any loop buffer or microop cache
; Uses different instructions to see if they fit in the microop cache
;
; Constants to be defined:
;
; tcase:     Test case:
;            1: loop with NOPs
;               Total bytes: 4 + (numinstr - 2) * nopsize (add 4 if > 127)
;            2: loop with short instructions, 2 bytes each
;               Total bytes: numinstr * 2 (add 4 if > 127)
;            3: loop with long instructions with big constants, 10 bytes each
;               Total bytes: numinstr * 10 - 16 (add 4 if > 127)
;            4: loop with short instructions and a branch inside
;               Total bytes: numinstr * 2 (add 4 if > 127)
;            5: loop with long instructions (by prefixes) and a branch inside
;               Total bytes: numinstr * 10 - 28 (add 4 if > 127)
;            6: Same as 3 but 32-bit constants instead of 64-bit. 5 bytes
;            7: Same as 6 but extended with redundant prefixes to 10 bytes
;            8: Instructions with 64 bit constants, 10 bytes each
;            9: Instructions with 32 bit addr offset + 32 bit imm. constants, 10 bytes each
;           10: Instructions with  8 bit addr offset + 32 bit imm. constants, 10 bytes each
;           11: Instructions with 32 bit addr offset +  8 bit imm. constants, 10 bytes each
;           12: Instructions with  8 bit addr offset +  8 bit imm. constants, 10 bytes each
;           13: Instructions with 32 bit addr offset + 16 bit imm. constants, 10 bytes each
;           14: Instructions with  8 bit addr offset + 16 bit imm. constants, 10 bytes each
;           15: Instructions with rip relative addr  + 16 bit imm. constants, 10 bytes each
;           16: Instructions with rip relative addr  + 32 bit imm. constants, 10 bytes each
;           17: Instructions with rip relative addr (big) + 32 bit imm. constants, 10 bytes each
;           18: Instructions with 64 bit abs address, 10 bytes each
;           19: One inst. w. 64 bit constant and three with 32 bits, 10 bytes each
;           20: Instructions with 32 bit absolute addr + 32 bit imm. constants, 10 bytes each
;           21: Instructions with 32 bit absolute addr + 16 bit imm. constants, 10 bytes each
;           22: Instructions with 32 bit absolute addr +  8 bit imm. constants, 10 bytes each
;           23: Instructions with 32 bit absolute addr +  0 bit imm. constants, 10 bytes each
;           24: Instructions with base + index + 0 bit offset + 32 bit imm. constants, 10 bytes each
;           25: Instructions with base + index + 8 bit offset + 16 bit imm. constants, 10 bytes each
;           26: Instructions with base + index + 32 bit offset + 16 bit imm. constants, 10 bytes each
;           27: AVX instructions, 5 bytes each
;
; numinstr:  Total number of instructions in loop
; 
; nopsize:   Size of NOP instructions for tcase 1 (1 - 15)
; 
; noptype:   Type of NOPs for tcase 1
;            2: long NOPs (0F 1F ...)
;            3: 66 NOPs (simple NOP with up to 14 operand size prefixes)
; 
; repeatn:   Number of loop repetitions
;
; (c) Copyright 2013 - 2021 by Agner Fog. GNU General Public License www.gnu.org/licenses
;-----------------------------------------------------------------------------
; Define any undefined macros

; define nops of all lengths

%ifndef noptype
   %define noptype 2
%endif

%include "nops.inc"


%macro NOPn 0
   %if nopsize == 1
      nop1
   %elif nopsize == 2
      nop2
   %elif nopsize == 3
      nop3
   %elif nopsize == 4
      nop4
   %elif nopsize == 5
      nop5
   %elif nopsize == 6
      nop6
   %elif nopsize == 7
      nop7
   %elif nopsize == 8
      nop8
   %elif nopsize == 9
      nop9
   %elif nopsize == 10
      nop10
   %elif nopsize == 11
      nop11
   %elif nopsize == 12
      nop12
   %elif nopsize == 13
      nop13
   %elif nopsize == 14
      nop14
   %elif nopsize == 15
      nop15
   %else
      %error unsupported nopsize
   %endif
%endmacro

%macro segprefixes 1
   times %1 db 3Eh
%endmacro


; Define each test case

%if tcase == 1  ; loop with NOPs

   %macro testcode 0
      mov ecx, repeatn
      align 16
      LL1:
      %if numinstr > 2
         %rep numinstr - 2
            NOPn
         %endrep
      %endif
      dec ecx
      jnz LL1
   %endmacro

%elif tcase == 2   ; loop with short instructions, 2 bytes each

   %macro testcode 0
      mov ecx, repeatn
      align 16
      LL1:
      %if numinstr > 2
         %rep (numinstr - 2) / 4
            mov eax,[psi]
            inc ebx
            inc edx
            mov edi, edi
         %endrep
         %rep (numinstr - 2) % 4
            mov eax,ebp
         %endrep         
      %endif
      dec ecx
      jnz LL1
   %endmacro

%elif tcase == 3   ; loop with long instructions, 10 bytes each

   %macro testcode 0
      mov ecx, repeatn
      align 16
      LL1:
      %if numinstr > 2
         %rep (numinstr - 2) / 4
            cmp dword [psi + 200h], 12345678
            mov rax,123456789ABCDEF0h
            mov rbx,123456789ABCDEF0h
            mov rdx,123456789ABCDEF0h
         %endrep
         %rep (numinstr - 2) % 4
            mov rax,123456789ABCDEF0h
         %endrep         
      %endif
      dec ecx
      jnz LL1
   %endmacro

%elif tcase == 4   ; loop with short instructions and a branch inside

   %macro testcode 0
      mov ecx, repeatn
      mov ebp, 1
      align 16
      LL1:
      %if numinstr > 5
         %rep (numinstr - 5) / 8
            mov eax,[psi]
            inc ebx
            inc edx
            mov edi, edi
         %endrep
         test ecx,ebp
         jz LL2
            mov edi, edi
         jnz LL3
         LL2:
            cmp edi, edi
         LL3:
         %rep (numinstr - ((numinstr - 5) / 8) * 4) / 4
            mov eax,[psi]
            inc ebx
            inc edx
            mov edi, edi
         %endrep
         %rep (numinstr - 2) % 4
            mov eax,ebp
         %endrep         
      %endif
      dec ecx
      jnz LL1
   %endmacro

%elif tcase == 5   ; loop with long instructions and a branch inside

   %macro testcode 0
      mov ecx, repeatn
      align 16
      LL1:
      %if numinstr > 5
         %rep (numinstr - 5) / 8
            segprefixes 6
            cmp byte [psi + 4h], 12
            segprefixes 5
            mov eax,12345678h
            segprefixes 5
            mov ebx,12345678h
            segprefixes 5
            mov edx,12345678h
         %endrep
         test ecx,1
         jz LL2
            segprefixes 5
            mov ebx,12345678h
         jnz LL3
         LL2:
            segprefixes 5
            mov ebx,12345678h
         LL3:
         %rep (numinstr - ((numinstr - 5) / 8) * 4) / 4
            segprefixes 6
            cmp byte [psi + 4h], 12
            segprefixes 5
            mov eax,12345678h
            segprefixes 5
            mov ebx,12345678h
            segprefixes 5
            mov edx,12345678h
         %endrep
         %rep (numinstr - 2) % 4
            segprefixes 5
            mov eax,12345678h
         %endrep         
      %endif
      dec ecx
      jnz LL1
   %endmacro
   
%elif tcase == 6   ; loop with medium instructions, 5 bytes

   %macro testcode 0
      mov ecx, repeatn
      align 16
      LL1:
      %if numinstr > 2
         %rep (numinstr - 2) / 4
            segprefixes 1
            cmp byte [psi + 4h], 12
            mov eax,12345678h
            mov ebx,12345678h
            mov edx,12345678h
         %endrep
         %rep (numinstr - 2) % 4
            mov eax,12345678h
         %endrep         
      %endif
      dec ecx
      jnz LL1
   %endmacro

%elif tcase == 7   ; loop with medium instructions with redundant prefixes up to 10

   %macro testcode 0
      mov ecx, repeatn
      align 16
      LL1:
      %if numinstr > 2
         %rep (numinstr - 2) / 4         
            segprefixes 6
            cmp byte [psi + 4h], 12
            segprefixes 5
            mov eax,12345678h
            segprefixes 5
            mov ebx,12345678h
            segprefixes 5
            mov edx,12345678h
         %endrep
         %rep (numinstr - 2) % 4
            segprefixes 5
            mov eax,12345678h
         %endrep         
      %endif
      dec ecx
      jnz LL1
   %endmacro
   
%elif tcase == 8   ; instructions with 64 bit constants, 10 bytes each

   %macro testcode 0
      mov ecx, repeatn
      align 16
      LL1:
      %if numinstr > 2
         %rep (numinstr - 2) / 4         
            mov rax,123456789ABCDEF0h
            mov rbx,123456789ABCDEF0h
            mov rdx,123456789ABCDEF0h
            mov rdi,123456789ABCDEF0h
         %endrep
         %rep (numinstr - 2) % 4
            mov rbx,123456789ABCDEF0h
         %endrep         
      %endif
      dec ecx
      jnz LL1
   %endmacro
   
%elif tcase == 9   ; instructions with two 32 bit constants, 10 bytes each

   %macro testcode 0
      mov ecx, repeatn
      align 16
      LL1:
      %if numinstr > 2
         %rep (numinstr - 2) / 4         
            cmp dword [psi + 200h], 12345678
            cmp dword [psi + 204h], 12345678
            mov dword [psi + 220h], 12345678
            nop10
         %endrep
         %rep (numinstr - 2) % 4
            cmp dword [psi + 204h], 12345678
         %endrep         
      %endif
      dec ecx
      jnz LL1
   %endmacro
   
%elif tcase == 10   ; Instructions with 32 bit constants, 8 bit offset, 10 bytes each

   %macro testcode 0
      mov ecx, repeatn
      align 16
      LL1:
      %if numinstr > 2
         %rep (numinstr - 2) / 4         
            segprefixes 3
            cmp dword [psi + 4], 12345678
            segprefixes 3
            cmp dword [psi + 8], 12345678
            segprefixes 3
            mov dword [psi + 16], 12345678
            nop10
         %endrep
         %rep (numinstr - 2) % 4
            segprefixes 3
            cmp dword [psi + 4], 12345678
         %endrep         
      %endif
      dec ecx
      jnz LL1
   %endmacro

%elif tcase == 11   ; Instructions with 8 bit constants, 32 bit offset, 10 bytes each

   %macro testcode 0
      mov ecx, repeatn
      align 16
      LL1:
      %if numinstr > 2
         %rep (numinstr - 2) / 4         
            segprefixes 3
            cmp byte [psi + 104h], 12
            segprefixes 3
            cmp byte [psi + 108h], 12
            segprefixes 3
            mov byte [psi + 110h], 12
            nop10
         %endrep
         %rep (numinstr - 2) % 4
            segprefixes 3
            cmp byte [psi + 104h], 12
         %endrep         
      %endif
      dec ecx
      jnz LL1
   %endmacro
   
%elif tcase == 12   ; Instructions with 8 + 8 bit constants, 10 bytes each

   %macro testcode 0
      mov ecx, repeatn
      align 16
      LL1:
      %if numinstr > 2
         %rep (numinstr - 2) / 4         
            segprefixes 6
            cmp byte [psi + 4], 12
            segprefixes 6
            cmp byte [psi + 8], 12
            segprefixes 6
            mov byte [psi + 16], 12
            nop10
         %endrep
         %rep (numinstr - 2) % 4
            segprefixes 6
            cmp byte [psi + 4], 12
         %endrep         
      %endif
      dec ecx
      jnz LL1
   %endmacro
   
%elif tcase == 13   ; Instructions with 16 bit constants, 32 bit offset, 10 bytes each

   %macro testcode 0
      mov ecx, repeatn
      align 16
      LL1:
      %if numinstr > 2
         %rep (numinstr - 2) / 4         
            segprefixes 2
            cmp word [psi + 104h], 12345
            segprefixes 2
            cmp word [psi + 108h], 12345
            segprefixes 2
            mov word [psi + 110h], 12345
            nop10
         %endrep
         %rep (numinstr - 2) % 4
            segprefixes 2
            cmp word [psi + 104h], 12345
         %endrep         
      %endif
      dec ecx
      jnz LL1
   %endmacro
   
%elif tcase == 14   ; Instructions with 16 bit constants, 8 bit offset, 10 bytes each

   %macro testcode 0
      mov ecx, repeatn
      align 16
      LL1:
      %if numinstr > 2
         %rep (numinstr - 2) / 4         
            segprefixes 5
            cmp word [psi + 4h], 12345
            segprefixes 5
            cmp word [psi + 8h], 12345
            segprefixes 5
            mov word [psi + 10h], 12345
            nop10
         %endrep
         %rep (numinstr - 2) % 4
            segprefixes 5
            cmp word [psi + 4h], 12345
         %endrep         
      %endif
      dec ecx
      jnz LL1
   %endmacro

%elif tcase == 15   ; Instructions with 16 bit constants, rip relative, 10 bytes each

   %macro testcode 0
      mov ecx, repeatn
      align 16
      LL1:
      %if numinstr > 2
         %rep (numinstr - 2) / 4         
            segprefixes 2
            cmp word [rel UserData], 12345
            segprefixes 2
            cmp word [rel UserData + 8h], 12345
            segprefixes 2
            mov word [rel UserData + 10h], 12345
            nop10
         %endrep
         %rep (numinstr - 2) % 4
            segprefixes 2
            cmp word [rel UserData], 12345
         %endrep         
      %endif
      dec ecx
      jnz LL1
   %endmacro
   
%elif tcase == 16 || tcase == 17  ; Instructions with 32 bit constants, rip relative, 10 bytes each

   %macro testdata 0
      %if tcase == 16
         times 100H  DB 0
      %elif tcase == 17
         times 1000000H  DB 0  ; 16 MB to make rip offset bigger
      %endif
   %endmacro   

   %macro testcode 0
      mov ecx, repeatn
      align 16
      LL1:
      %if numinstr > 2
         %rep (numinstr - 2) / 4         
            cmp dword [rel UserData], 12345
            cmp dword [rel UserData + 8h], 12345
            mov dword [rel UserData + 10h], 12345
            nop10
         %endrep
         %rep (numinstr - 2) % 4
            cmp dword [rel UserData], 12345
         %endrep         
      %endif
      dec ecx
      jnz LL1
   %endmacro
   
   
%elif tcase == 18   ; instructions with 64 bit address, 10 bytes each

   %macro testcode 0
      mov ecx, repeatn
      align 16
      LL1:
      %if numinstr > 2
         %rep (numinstr - 2) / 4         
            mov rax,[abs qword UserData]
            mov rax,[abs qword UserData+8]
            segprefixes 7            
            xor rax,rax
            mov [abs qword UserData+20h],rax
         %endrep
         %rep (numinstr - 2) % 4
            mov rax,[abs qword UserData]
         %endrep         
      %endif
      dec ecx
      jnz LL1
   %endmacro
   
%elif tcase == 19   ; One inst. w. 64 bit constant and three with 32 bits, 10 bytes each

   %macro testcode 0
      mov ecx, repeatn
      align 16
      LL1:
      %if numinstr > 2
         %rep (numinstr - 2) / 4         
            mov rax,123456789ABCDEF0h
            segprefixes 5
            mov ebx,12345678h
            segprefixes 6
            cmp byte [psi + 4h], 12
            segprefixes 5
            mov edx,12345678h
         %endrep
         %rep (numinstr - 2) % 4
            mov ebx,12345678h
            segprefixes 6
         %endrep         
      %endif
      dec ecx
      jnz LL1
   %endmacro
   
   
%elif tcase == 20   ; Instructions with 32 bit absolute addr + 32 bit imm. constants, 10 bytes each   
   
   %macro testcode 0
      mov ecx, repeatn
      align 16
      LL1:
      %if numinstr > 2
         %rep (numinstr - 2) / 4         
            cmp dword [abs UserData], 12345
            cmp dword [abs UserData + 8h], 12345
            mov dword [abs UserData + 10h], 12345
            nop10
         %endrep
         %rep (numinstr - 2) % 4
            cmp dword [abs UserData], 12345
         %endrep         
      %endif
      dec ecx
      jnz LL1
   %endmacro

%elif tcase == 21   ; Instructions with 32 bit absolute addr + 16 bit imm. constants, 10 bytes each   
   
   %macro testcode 0
      mov ecx, repeatn
      align 16
      LL1:
      %if numinstr > 2
         %rep (numinstr - 2) / 4         
            segprefixes 2
            cmp word [abs UserData], 12345
            segprefixes 2
            cmp word [abs UserData + 8h], 12345
            segprefixes 2
            mov word [abs UserData + 10h], 12345
            nop10
         %endrep
         %rep (numinstr - 2) % 4
            segprefixes 2
            cmp word [abs UserData], 12345
         %endrep         
      %endif
      dec ecx
      jnz LL1
   %endmacro
   
%elif tcase == 22   ; Instructions with 32 bit absolute addr + 8 bit imm. constants, 10 bytes each   
   
   %macro testcode 0
      mov ecx, repeatn
      align 16
      LL1:
      %if numinstr > 2
         %rep (numinstr - 2) / 4         
            segprefixes 3
            cmp byte [abs UserData], 123
            segprefixes 3
            cmp byte [abs UserData + 8h], 123
            segprefixes 3
            mov byte [abs UserData + 10h], 123
            nop10
         %endrep
         %rep (numinstr - 2) % 4
            segprefixes 3
            cmp byte [abs UserData], 123
         %endrep         
      %endif
      dec ecx
      jnz LL1
   %endmacro 
   
%elif tcase == 23   ; Instructions with 32 bit absolute addr + 0 bit imm. constants, 10 bytes each   
   
   %macro testcode 0
      mov ecx, repeatn
      align 16
      LL1:
      %if numinstr > 2
         %rep (numinstr - 2) / 4         
            segprefixes 4
            cmp dword [abs UserData], edi
            segprefixes 4
            cmp dword [abs UserData + 8h], ebx
            segprefixes 4
            mov dword [abs UserData + 10h], edx
            nop10
         %endrep
         %rep (numinstr - 2) % 4
            segprefixes 4
            cmp dword [abs UserData], ebx
         %endrep         
      %endif
      dec ecx
      jnz LL1
   %endmacro 
   
%elif tcase == 24   ; Instructions with base + index + 0 bit offset + 32 bit imm. constants, 10 bytes each   

   %macro testinit1 0
     lea r8, [UserData]
     lea r9, [UserData+4]
     lea r10, [UserData+10h]
     xor edi,edi
   %endmacro 

   %macro testcode 0
      mov ecx, repeatn
      align 16
      LL1:
      %if numinstr > 2
         %rep (numinstr - 2) / 4         
            segprefixes 2
            cmp dword [r8+4*rdi], 12345
            segprefixes 2
            cmp dword [r9+4*rdi], 12345
            segprefixes 2
            mov dword [r10+4*rdi], 12345
            nop10
         %endrep
         %rep (numinstr - 2) % 4
            segprefixes 2
            cmp dword [r8+4*rdi], 12345
         %endrep         
      %endif
      dec ecx
      jnz LL1
   %endmacro
   
%elif tcase == 25   ; Instructions with base + index + 8 bit offset + 16 bit imm. constants, 10 bytes each

   %macro testinit1 0
     lea r8, [UserData]
     xor edi,edi
   %endmacro 

   %macro testcode 0
      mov ecx, repeatn
      align 16
      LL1:
      %if numinstr > 2
         %rep (numinstr - 2) / 4         
            segprefixes 4
            cmp word [r8+4*rdi+4], 12345
            segprefixes 4
            cmp word [r8+4*rdi+8], 12345
            segprefixes 4
            mov word [r8+4*rdi+10h], 12345
            nop10
         %endrep
         %rep (numinstr - 2) % 4
            segprefixes 4
            cmp word [r8+4*rdi+4], 12345
         %endrep         
      %endif
      dec ecx
      jnz LL1
   %endmacro
   
%elif tcase == 26   ; Instructions with base + index + 32 bit offset + 16 bit imm. constants, 10 bytes each

   %macro testinit1 0
     lea rsi, [UserData]
     xor edi,edi
   %endmacro 

   %macro testcode 0
      mov ecx, repeatn
      align 16
      LL1:
      %if numinstr > 2
         %rep (numinstr - 2) / 4         
            segprefixes 1
            cmp word [rsi+4*rdi+104h], 12345
            segprefixes 1
            cmp word [rsi+4*rdi+108h], 12345
            segprefixes 1
            mov word [rsi+4*rdi+110h], 12345
            nop10
         %endrep
         %rep (numinstr - 2) % 4
            segprefixes 1
            cmp word [rsi+4*rdi+104h], 12345
         %endrep         
      %endif
      dec ecx
      jnz LL1
   %endmacro

%elif tcase == 27   ; AVX instructions

   %macro testcode 0
      mov ecx, repeatn
      align 16
      LL1:
      %if numinstr > 2
         %rep (numinstr - 2) / 4         
            vaddps xmm0,xmm8,[psi+20h]
            vpaddd xmm2,xmm9,[psi+40h]
            vpor xmm10,xmm11
            vmovdqa [psi+60h],xmm12
         %endrep
         %rep (numinstr - 2) % 4
            vpaddd xmm2,xmm9,[psi+40h]
         %endrep         
      %endif
      dec ecx
      jnz LL1
   %endmacro

%else
   %error unknown test case tcase
%endif


; template loops
%define repeat1 1
%define repeat2 1


;##############################################################################
;#
;#                 Extra calculations for convenience
;#
;##############################################################################

%macro extraoutput 0
Heading1        DB   "instr/clock", 0
Heading2        DB   "bytes/clock", 0
align 8

; Decide which column to base clock count and uop count on
%ifidni CPUbrand,Intel
%define ClockCol  1   ; use core clock cycles on Intel processors
%define InstCol   2   ; Instruction count
%define UopCol    4   ; 3 or 4, which has the best uop count?

%else                 ; All other CPU brands:
%define ClockCol  0   ; use RDTSC clock on all other processors
%define InstCol   1   ; Instruction count
%define UopCol    2   ; uop count
%endif

RatioOut        DD   2           ; 0: no ratio output, 1 = int, 2 = float
                DD   InstCol     ; numerator (0 = clock, 1 = first PMC, etc., -1 = none)
                DD   ClockCol    ; denominator (0 = clock, 1 = first PMC, etc., -1 = none)
                DD   1.0         ; factor, int or float according to RatioOut[0]
                
TempOut         DD   8           ; 8 = float, corrected for clock factor
                DD   0
%if modesize > 32
  RatioOutTitle DQ   Heading1      ; column heading
  TempOutTitle  DQ   Heading2      ; column heading                
%else 	
  RatioOutTitle DD   Heading1      ; column heading
  TempOutTitle  DD   Heading2      ; column heading                
%endif
                
%endmacro       


%macro testafter3 0
; calculate bytes per clock
%if modesize > 32

    xor     r14d, r14d
TESTAFTERLOOP:

    mov      eax, repeat1 * (repeat2 * repeatn * numinstr * nopsize + 8)                       ; calculate number of bytes
    cvtsi2ss xmm0, eax
    mov      ebx, [r13 + r14*4 + (ClockCol-1)*4*REPETITIONS0+(PMCResults-ThreadData)] ; recall clock count
    cvtsi2ss xmm1, ebx
    divss    xmm0, xmm1                                                              ; divide
    movss    [r13 + r14*4 + (CountTemp-ThreadData)], xmm0                            ; store in CountTemp

    inc     r14d
    cmp     r14d, REPETITIONS0
    jb      TESTAFTERLOOP

%endif
%endmacro 
