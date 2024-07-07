;----------------------------------------------------------------------------
;                       memory_mirror.inc 
;
; testing the mirroring of memory operands.
; The AMD Zen 2 CPU can mirror memory operands in temporary registers under certain
; conditions, tested here
;                           NASM syntax
;
; Last modified: 2021-03-23
;
; The following macros can be defined on the command line or in include files:
; 
; tcase:    Test case:
; WRL       write to read latency with same pointer
; WRT       write to read throughput with same pointer
; WRML      write / read-modify latency with same address
; RMWL      read-modify-write instruction latency with same address
; WRArip    write / read latency with rip-relative address
; WRAabs    write / read latency with absolute address
; WRAabsDUMMYP write / read latency with absolute address and dummy zero pointer
; WRPO8     write / read latency with pointer and 8 bit offset
; WRPO8I    write / read latency with pointer and 8 bit offset and index
; WRPO10P   write / read latency with pointer and offset fitting 10 bits
; WRPO16P   write / read latency with pointer and offset fitting 16 bits
; WRPO16PI  write / read latency with pointer and offset fitting 16 bits and scaled index
; WRPO24P   write / read latency with pointer and offset fitting 24 bits
; WRPO32P   write / read latency with pointer and offset fitting 32 bits
; WRPOMISALIGN   write / read latency with offset not divisible by 4 or 8
; WRPISCALE write / read latency with pointer and scaled index
; WRPISCALEDIF0 write / read latency with pointer and different scale factors. index = 0
; WRPISCALEDIF1 write / read false dependence with different scale factors. index != 0
; WRPA      write / read latency with aliased pointer
; WRPIS     write / read latency with pointer and index registers swapped
; WRPUSH    write / read latency with push/pop
; WRFUNC    write / read latency with function parameter on stack
; WRFUNCBP  write / read latency with parameter on stack and rbp address
; WRFUNCSPSUB write / read latency with parameter on stack and frame size subtracted from rsp
; PUSHO8    write / read latency with stack push causing 8 bit offset
; PUSHO32   write / read latency with stack push causing 32 bit offset
; PALIASWWR write / write / read with pointer aliasing. possible penalty
; PALIASWWR2 write / write / read with pointer and rip relative address. possible penalty
; PALIASWMR write / read-modify-write / read with pointer aliasing. possible penalty
; 
;
; (c) Copyright 2020-2021 by Agner Fog. GNU General Public License www.gnu.org/licenses
;-----------------------------------------------------------------------------
; Define any undefined macros

%ifndef tcase
   %define tcase WRL
%endif

%ifndef AOFFSET
   %define AOFFSET 4
%endif

%macro testinit2 0
%endmacro

%macro testinit3 0   ; needs to reset rsi if lodsb instruction used
%endmacro

%macro testafter3 0
%endmacro 

%macro testdata 0
        times 10000H  DB 0
%endmacro

; Define registers depending on regtype and regsize
%ifndef regtype
   %ifndef regsize
      %define regsize 32
   %endif
   %if regsize == 9
      %define regtype h
   %elif regsize < 65
      %define regtype r
   %elif regsize == 65
      %define regtype mmx
   %else
      %define regtype v
   %endif
%endif

; define move instruction for selected register type
%ifidni regtype, r
  %define moveinstr mov
%elifidni regtype, mmx
  %define moveinstr movq
%elifidni regtype, v
  %if regsize <= 128
    %define moveinstr movaps
  %else 
    %define moveinstr vmovaps
  %endif
%else
%error unknown register type
%endif





;##############################################################################
;#
;#                 Test code macro
;#
;##############################################################################

; main testcode macro
%macro testcode 0


%ifidni tcase, WRL  ; write to read latency with same pointer

mov ebp, 100
align 32
LL:
%rep 100
  moveinstr [rsi], reg1
  moveinstr reg1, [rsi]
%endrep
dec ebp
jnz LL

%elifidni tcase, WRT  ; write to read throughput with same pointer

mov ebp, 100
lea r8, [rsi + 0x100]
align 32
LL:
%rep 25
  moveinstr [rsi], reg1
  moveinstr [rsi+64], reg2
  moveinstr [r8], reg3
  moveinstr [r8+64], reg4
  moveinstr reg1, [rsi]
  moveinstr reg2, [rsi+64]
  moveinstr reg3, [r8]
  moveinstr reg4, [r8+64]
%endrep
dec ebp
jnz LL

%elifidni tcase, WRML   ;   write / read-modify latency with same address

mov ebp, 100
align 32
LL:
%rep 100
  moveinstr [rsi], reg1
  add reg1, [rsi]
%endrep
dec ebp
jnz LL

%elifidni tcase, RMWL   ;   read-modify-write instruction latency with same address

mov ebp, 100
align 32
LL:
%rep 100
  add [rsi], reg1
%endrep
dec ebp
jnz LL

%elifidni tcase, WRArip    ; write / read latency with rip-relative address

mov ebp, 100
align 32
LL:
%rep 100
  moveinstr [rel UserData], reg1
  moveinstr reg1, [rel UserData]
%endrep
dec ebp
jnz LL

%elifidni tcase, WRAabs    ; write / read latency with absolute address

mov ebp, 100
align 32
LL:
%rep 100
  moveinstr [abs UserData], reg1
  moveinstr reg1, [abs UserData]
%endrep
dec ebp
jnz LL

%elifidni tcase, WRAabsDUMMYP   ; write / read latency with absolute address and dummy zero pointer    ; write / read latency with absolute address

mov ebp, 100
xor edi, edi
align 32
LL:
%rep 100
  moveinstr [abs UserData + edi], reg1
  moveinstr reg1, [abs UserData + edi]
%endrep
dec ebp
jnz LL

%elifidni tcase, WRPO8     ; write / read latency with pointer and 8 bit offset

mov ebp, 100
align 32
LL:
%rep 100
  moveinstr [rsi+0x70], reg1
  moveinstr reg1, [rsi+0x70]
%endrep
dec ebp
jnz LL

%elifidni tcase, WRPO8I    ; write / read latency with pointer and 8 bit offset and index

mov ebp, 100
xor edi, edi
align 32
LL:
%rep 100
  moveinstr [rsi+0x70+rdi], reg1
  moveinstr reg1, [rsi+0x70+rdi]
%endrep
dec ebp
jnz LL


%elifidni tcase, WRPO10P  ; write / read latency with pointer and offset fitting 10 bits

mov ebp, 100
align 32
LL:
%rep 100
  moveinstr [rsi+0x7f0], reg1
  moveinstr reg1, [rsi+0x7f0]
%endrep
dec ebp
jnz LL

%elifidni tcase, WRPO16P  ; write / read latency with pointer and offset fitting 16 bits

mov ebp, 100
align 32
LL:
%rep 100
  moveinstr [rsi+0x4000], reg1
  moveinstr reg1, [rsi+0x4000]
%endrep
dec ebp
jnz LL

%elifidni tcase, WRPO16PI  ; write / read latency with pointer and offset fitting 16 bits and scaled index

mov ebp, 100
xor edi, edi
align 32
LL:
%rep 100
  moveinstr [rsi+rdi+0x4000], reg1
  moveinstr reg1, [rsi+rdi+0x4000]
%endrep
dec ebp
jnz LL

%elifidni tcase, WRPO24P  ;  write / read latency with pointer and offset fitting 24 bits

mov ebp, 100
lea rdi, [rsi-0x700000]
align 32
LL:
%rep 100
  moveinstr [rdi+0x700000], reg1
  moveinstr reg1, [rdi+0x700000]
%endrep
dec ebp
jnz LL

%elifidni tcase, WRPO32P    ; write / read latency with pointer and 32 bit offset

mov ebp, 100
lea rdi, [rsi-0x10000000]
align 32
LL:
%rep 100
  moveinstr [rdi+0x10000000], reg1
  moveinstr reg1, [rdi+0x10000000]
%endrep
dec ebp
jnz LL

%elifidni tcase, WRPO32PI    ; write / read latency with pointer and scaled index and 32 bit offset

mov ebp, 100
lea rdi, [rsi-0x10000000]
mov r8d, 2
align 32
LL:
%rep 100
  moveinstr [rdi+r8*4+0x10000000], reg1
  moveinstr reg1, [rdi+r8*4+0x10000000]
%endrep
dec ebp
jnz LL

%elifidni tcase, WRPOMISALIGN ; write / read latency with offset not divisible by 4 or 8

%if modesize == 32 
%define pdi edi
%else 
%define pdi rdi
%endif

mov ebp, 100
lea pdi, [psi+60]
align 32
LL:
%rep 100
  moveinstr [pdi+AOFFSET], reg1
  moveinstr reg1, [pdi+AOFFSET]
%endrep
dec ebp
jnz LL

%elifidni tcase, WRPISCALE   ; write / read latency with pointer and scaled index

mov ebp, 100
mov edi, 4
align 32
LL:
%rep 100
  moveinstr [rsi+rdi*4], reg1
  moveinstr reg1, [rsi+rdi*4]
%endrep
dec ebp
jnz LL

%elifidni tcase, WRPISCALEDIF0   ; write / read latency with pointer and different scale factors. index = 0

mov ebp, 100
mov edi, 0
align 32
LL:
%rep 100
  moveinstr [rsi+rdi*2], reg1
  moveinstr reg1, [rsi+rdi*8]
%endrep
dec ebp
jnz LL

%elifidni tcase, WRPISCALEDIF1   ; write / read false dependence with different scale factors. index != 0

mov ebp, 100
mov edi, 4
align 32
LL:
%rep 100
  moveinstr [rsi+rdi*2], reg1
  moveinstr reg1, [rsi+rdi*8]
%endrep
dec ebp
jnz LL

%elifidni tcase, WRPA      ; write / read latency with aliased pointer

mov ebp, 100
mov rdi, rsi
align 32
LL:
%rep 100
  moveinstr [rsi], reg1
  moveinstr reg1, [rdi]
%endrep
dec ebp
jnz LL

%elifidni tcase, WRPIS     ; write / read latency with pointer and index registers swapped

mov ebp, 100
mov edi, 8
align 32
LL:
%rep 100
  moveinstr [rsi+rdi], reg1
  moveinstr reg1, [rdi+rsi]
%endrep
dec ebp
jnz LL

%elifidni tcase, WRPUSH    ; write / read latency with push/pop

mov ebp, 100
align 32
LL:
%rep 100
  push reg1
  pop reg1
%endrep
dec ebp
jnz LL

%elifidni tcase, WRFUNC    ; write / read latency with function parameter on stack

mov ebp, 100
align 32
LL:
%rep 100
  push reg1
  call FUNC1
  pop reg2
%endrep
dec ebp
jnz LL

jmp EEE

FUNC1: 
%if modesize == 32
mov reg1, [esp+4]
%else
mov reg1, [rsp+8]
%endif
ret

EEE:

%elifidni tcase, WRFUNCBP  ; write / read latency with parameter on stack and rbp address

mov r10d, 100
align 32
LL:
%rep 100
  push reg1
  call FUNC1
  pop reg2
%endrep
dec r10d
jnz LL

jmp EEE

FUNC1: 
mov rbp, rsp
mov reg1, [rbp+8]
ret

EEE:

%elifidni tcase, WRFUNCSPSUB ; write / read latency with parameter on stack and frame size subtracted from rsp

mov r10d, 100
align 32
LL:
%rep 100
  push reg1
  call FUNC1
  pop reg2
%endrep
dec r10d
jnz LL

jmp EEE

FUNC1: 
sub rsp, 8
mov reg1, [rsp+8+8]
add rsp, 8
ret
EEE:

%elifidni tcase, PUSHO8    ; write / read latency with stack push causing 8 bit offset

%assign UU 0x70
mov ebp, 100
mov edi, 0
align 32
LL:
%rep 100
  mov [rsp+UU],eax
  push rbx
  mov [rsp+UU+8],edx
  pop rcx
  mov eax,[rsp+UU]
%endrep
dec ebp
jnz LL

%elifidni tcase, PUSHO32   ; write / read latency with stack push causing 32 bit offset

%assign UU 0x78
mov ebp, 100
mov edi, 0
align 32
LL:
%rep 100
  mov [rsp+UU],eax
  push rbx
  mov [rsp+UU+8],edx
  pop rcx
  mov eax,[rsp+UU]
%endrep
dec ebp
jnz LL

%elifidni tcase, PALIASWWR ; write / write / read with pointer aliasing. possible penalty

mov ebp, 100
mov rdi, rsi
align 32
LL:
%rep 100
  mov [rsi],reg1
  mov [rdi],reg2
  mov reg1, [rsi]
%endrep
dec ebp
jnz LL

%elifidni tcase, PALIASWWR2 ; write / write / read with pointer and rip relative address. possible penalty

mov ebp, 100
lea rsi, [UserData]
align 32
LL:
%rep 100
  mov [rsi],reg1
  mov [UserData],reg2
  mov reg1, [rsi]
%endrep
dec ebp
jnz LL

%elifidni tcase, PALIASWMR  ; write / read-modify-write / read with pointer aliasing. possible penalty

mov ebp, 100
mov rdi, rsi
align 32
LL:
%rep 100
  mov [rsi],reg1
  add [rdi],reg2
  mov reg1, [rsi]
%endrep
dec ebp
jnz LL


%else

%error unknown test case tcase

%endif

%endmacro    ; testcode


;  default test loops
%define repeat1 1
%define repeat2 1

