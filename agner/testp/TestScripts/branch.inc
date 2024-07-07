;----------------------------------------------------------------------------
;                       branch.inc                       2013-04-26 Agner Fog
;
;            PMC Test program for testingbranch prediction
;                           NASM syntax
;
; The following macros can be defined on the command line or in include files:
; 
; case:     Test case number. See below for each case
; 
; count1:   Loop count for outer loop
; 
; count2:   Loop count for inner loop
;
;
; (c) Copyright 2013 by Agner Fog. GNU General Public License www.gnu.org/licenses
;-----------------------------------------------------------------------------

%ifndef case
   %define case  1          ; default case 1
%endif

%ifndef count1
   %define count1   10      ; default count1
%endif

%ifndef count2
   %define count2   16      ; default count2
%endif


;##############################################################################
;#
;#                 Test code macros
;#
;##############################################################################


%macro localloop 1
; make a loop with count %1
mov eax, %1
%%L1: 
  nop
  dec eax
  jnz %%L1
nop
%endmacro


; main testcode macro
%macro testcode 0

%if case == 1         ; case 1: consecutive simple loops

%REP count1
localloop count2
%ENDREP

%elif case == 2       ; case 2: nested loops

mov ebx, count1
align 16
LL:
  nop
  localloop count2
  nop
  dec ebx
  jnz LL
nop

%elif case == 3       ; case 3: loop with 1 alternating branch inside

mov edi, count1
xor eax, eax
align 16
LL:
  nop
  not eax
  jnz L1
  nop
  L1:
  nop
dec edi
jnz LL  
  
%elif case == 4       ; case 4: loop with 2 alternating branches inside

mov edi, count1
xor eax, eax
align 16
LL:
  nop
  not eax
  jnz L1
  nop
  L1:
  nop
  jz L2
  nop
  L2:
  nop  
dec edi
jnz LL  
  
%elif case == 5       ; case 5: Loop with one branch having simple repetitive pattern of period count2 inside

   %if count2 > 5
   %define counttak  (count2 / 2 - 1)
   %else
   %define counttak  (count2 / 2)
   %endif

mov edi, count1
xor eax, eax
xor edx, edx
align 16
LL:
  nop
  inc eax
  cmp eax, counttak
  jna L1
  nop
  L1:
  nop
  cmp eax, count2
  cmovae eax, edx
dec edi
jnz LL

%elif case == 6       ; case 6: Loop with one branch having complicated repetitive pattern of period count2 inside

   %if count2 > 5
   mov rbx, 0xEB271F490D83CA56
   %else
   mov rbx, 0xEB271F490D83CA65
   %endif

mov edi, count1
xor eax, eax
xor edx, edx
align 16
LL:
  nop
  ror rbx,1
  jc L1
  nop
  L1:
  nop
  cmp eax, count2
  cmovae eax, edx
dec edi
jnz LL
  
%elif case == 7       ; case 7: loop with one 0011 branch and one 00011 branch inside

mov edi, count1
xor eax, eax
xor ebx, ebx
xor edx, edx
align 16
LL:
  nop
  inc eax
  test eax, 2
  jnz L1
  nop
  L1:
  nop
  inc ebx
  cmp ebx, 2
  ja L2
  nop
  L2:
  cmp ebx, 5
  cmovae ebx, edx
dec edi
jnz LL

%elif case == 8       ; case 8: Loop with alternating indirect branch inside

mov edi, count1
xor eax, eax
align 16
LL:
  nop
  inc eax
  test eax, 2
  lea rbx, [rel L1]
  lea rcx, [rel L2]
  cmovz rbx, rcx
  jmp rbx
  nop
  L1:
  nop
  nop
  nop
  nop
  nop
  nop
  nop
  nop
  L2:
  nop
  nop
  nop
dec edi
jnz LL
 
%elif case == 9       ; case 9: Loop with 3-way indirect branch inside

mov edi, count1
xor eax, eax
xor r8d, r8d
align 16
LL:
  nop
  inc eax
  lea rbx, [rel L1]
  lea rcx, [rel L2]
  lea rdx, [rel L3]
  cmp eax, 2
  cmovb rcx, rbx
  cmova rcx, rdx
  cmova eax, r8d
  nop
  jmp rcx
  nop
  L1:
  nop
  L2:
  nop
  L3:
  nop
  nop
dec edi
jnz LL

%endif


%endmacro   ; testcode

; disable default test loops
%define repeat1 1
%define repeat2 1

