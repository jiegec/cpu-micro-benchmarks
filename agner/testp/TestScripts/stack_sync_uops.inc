;----------------------------------------------------------------------------
;                       stack_sync_uops.inc        2013-07-21 Agner Fog
;
;            PMC Test program for testingbranch prediction
;                           NASM syntax
;
; The following macros can be defined on the command line or in include files:
; 
; tcase:    Test case number. See below for each case
;           1. Push and pop only
;           2. added mov r,[rsp]
;           3. further added mov r,rsp
;           4. call and ret
;           5. call and ret imm
;           6. call and ret and add rsp,const
;
;
; (c) Copyright 2013 by Agner Fog. GNU General Public License www.gnu.org/licenses
;-----------------------------------------------------------------------------

%ifndef tcase
   %define case  1          ; default case 1
%endif

%ifndef count1
   %define count1   10      ; default count1
%endif


;##############################################################################
;#
;#                 Test code macros
;#
;##############################################################################


%if tcase < 4   ; Push and pop only

%macro testcode 0
nop
nop
mov ebp, count1
align 16
LL:
     push rax
     push rbx
     push rcx
     %if tcase > 1
     mov r8,[rsp]
     %endif
     pop rdx
     pop rdx
     pop rdx
     %if tcase > 2
     mov rdi,rsp
     %endif
dec ebp
jnz LL
%endmacro

%elif tcase == 4 ; call and return

%macro testinit1 0
   jmp AROUND
   align 16
   TESTFUNC:
   nop
   nop
   nop
   ret
   align 16
   AROUND:
%endmacro

%macro testcode 0
mov ebp, count1
align 16
LL:
     push rax
     push rbx
     call TESTFUNC
     pop rdx
     pop rdx
dec ebp
jnz LL
%endmacro

%elif tcase == 5 ; call and ret imm

%macro testinit1 0
   jmp AROUND
   align 16
   TESTFUNC:
   nop
   nop
   nop
   ret 16
   align 16
   AROUND:
%endmacro

%macro testcode 0
mov ebp, count1
align 16
LL:
     push rax
     push rbx
     call TESTFUNC
dec ebp
jnz LL
%endmacro

%elif tcase == 6 ; call and ret and add rsp,const

%macro testinit1 0
   jmp AROUND
   align 16
   TESTFUNC:
   nop
   nop
   nop
   ret
   align 16
   AROUND:
%endmacro

%macro testcode 0
mov ebp, count1
align 16
LL:
     push rax
     push rbx
     call TESTFUNC
     add rsp,8
     pop rcx
dec ebp
jnz LL
%endmacro

%else
%error unknown test case tcase
%endif

; disable default test loops
%define repeat1 1
%define repeat2 1

