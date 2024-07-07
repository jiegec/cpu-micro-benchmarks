;----------------------------------------------------------------------------
;               ucache_double_entries.inc
;
; Program for testing if instructions use double entries in microop cache
; (Only for processors that have microop cache)
;
; Last modified: 2020-08-16
;
;                           NASM syntax
;
; The following macros can be defined on the command line or in include files:
; 
; tcase:    Test case number. See below for each case
; 
; repeat1:      Repeat count for loop around testcode. Default = no loop
;
; repeat2:      Repeat count for repeat macro around testcode. Default = 100
;
;
; (c) Copyright 2013-2020 by Agner Fog. GNU General Public License www.gnu.org/licenses
;-----------------------------------------------------------------------------

%ifndef case
   %define case  1          ; default case 1
%endif

%ifndef codealign
   %define codealign 32     ; align code by 32
%endif


;##############################################################################
;#
;#                 Test code macros
;#
;##############################################################################

; define long nops
%ifndef noptype
   %define noptype 2
%endif

%include "nops.inc"

%macro segprefixes 1
   times %1 db 3EH
%endmacro



; main testcode macro

%if tcase == 1         ; case 1: mov reg64,i32;  S-L-L-L

   %macro testcode 0
   nop2
   segprefixes 3
   mov rax,-100000h
   segprefixes 3
   mov rbx,-200000h
   segprefixes 3
   mov rcx,-300000h
   %endmacro

%elif tcase == 2       ; case 2: mov reg64,i64;  S-L-L-L

   %macro testcode 0
   nop2
   mov rax,123456789ABCDEF0H     ; 10 bytes
   mov rbx,123456789ABCDEF0H
   mov rcx,123456789ABCDEF0H
   %endmacro

%elif tcase == 3       ; case 3: mov reg64,i64;  L-L-L-S

   %macro testcode 0
   mov rax,123456789ABCDEF0H     ; 10 bytes
   mov rbx,123456789ABCDEF0H
   mov rcx,123456789ABCDEF0H
   nop2
   %endmacro

%elif tcase == 4       ; case 4: mov reg64,i64;  L-S-S-S

   %macro testcode 0
   mov rax,0000000100000000H     ; 10 bytes
   nop2
   nop10
   nop10
   %endmacro
   
%elif tcase == 5       ; case 5: mov reg64,i64small;  L-L-L-S

   %macro testcode 0
   ; mov rax,0000000000001000H     ; 10 bytes
   ; mov rbx,0000000000001000H     ; 10 bytes
   ; mov rcx,0000000000001000H     ; 10 bytes
   db 48h, 0B8h
   dd 1000H, 00000H
   db 48h, 0BBh
   dd 1000H, 00000H
   db 48h, 0B9h
   dd 1000H, 00000H
   nop2
   %endmacro

   ; case 6 - 11 are in 32-bit mode:

%elif tcase == 6       ; case 6: cmp [abs32],r;  L-L-L-S

   %macro testcode 0
   segprefixes 1
   cmp [abs dword UserData], eax
   segprefixes 1
   cmp [abs dword UserData+10h], ebx
   segprefixes 1
   cmp [abs dword UserData+20h], ecx
   nop11
   %endmacro

%elif tcase == 7       ; case 7: cmp [abs32],i8;  L-L-L-S

   %macro testcode 0
   cmp byte [abs dword UserData], 1
   cmp byte [abs dword UserData+10h], 2
   cmp byte [abs dword UserData+20h], 3
   nop11
   %endmacro

%elif tcase == 8       ; case 8: cmp [abs32],i16;  L-L-L-S

   %macro testcode 0
   cmp word [abs dword UserData], 1000h
   cmp word [abs dword UserData+10h], 2000h
   cmp word [abs dword UserData+20h], 3000h
   nop5
   %endmacro
   
%elif tcase == 9       ; case 9: cmp [abs32],i32;  L-L-L-S

   %macro testcode 0
   cmp dword [abs dword UserData], 100000h
   cmp dword [abs dword UserData+10h], 200000h
   cmp dword [abs dword UserData+20h], 300000h
   nop2
   %endmacro
   
%elif tcase == 10       ; case 10: cmp [abs32+p],i8;  L-L-L-S

   %macro testinit2 0
   xor ebx,ebx
   %endmacro

   %macro testcode 0
   cmp byte [abs UserData+ebx], 1
   cmp byte [abs UserData+10h+ebx], 2
   cmp byte [abs UserData+20h+ebx], 3
   nop11
   %endmacro

%elif tcase == 11       ; case 11: cmp [abs32+p],i32;  L-L-L-S

   %macro testinit2 0
   xor ebx,ebx
   %endmacro

   %macro testcode 0
   cmp dword [abs UserData+ebx], 100000h
   cmp dword [abs UserData+10h+ebx], 200000h
   cmp dword [abs UserData+20h+ebx], 300000h
   nop2
   %endmacro

%elif tcase == 12      ; case 12: mov [abs64];  L-L-L-S

   %macro testcode 0
   mov eax,[abs qword UserData]
   mov eax,[abs qword UserData+10h]
   mov [abs qword UserData+20h],eax
   nop5
   %endmacro

%elif tcase == 13      ; case 13: cmp [basep], i8;  L-L-L-S

   %macro testcode 0
   segprefixes 4
   cmp byte [rsi], 1
   segprefixes 4
   cmp byte [rdi], 2
   segprefixes 4
   cmp byte [rsi], 3
   nop11
   %endmacro
   
%elif tcase == 14      ; case 14: cmp [basep], i16;  L-L-L-S

   %macro testcode 0
   segprefixes 3
   cmp word [rsi], 1000h
   segprefixes 3
   cmp word [rdi], 2000h
   segprefixes 3
   cmp word [rsi], 3000h
   nop8
   %endmacro
   
%elif tcase == 15      ; case 15: cmp [basep], i32;  L-L-L-S

   %macro testcode 0
   segprefixes 1
   cmp dword [rsi], 100000h
   segprefixes 1
   cmp dword [rsi], 200000h
   segprefixes 1
   cmp dword [rsi], 300000h
   nop11
   %endmacro

%elif tcase == 16      ; case 16: cmp [basep+o8], i8;  L-L-L-S

   %macro testcode 0
   segprefixes 3
   cmp dword [rsi+10h], 1
   segprefixes 3
   cmp dword [rsi+20h], 2
   segprefixes 3
   cmp dword [rsi+30h], 3
   nop11
   %endmacro
   
%elif tcase == 17      ; case 17: cmp [basep+o8], i16;  L-L-L-S

   %macro testcode 0
   segprefixes 2
   cmp word [rsi+10h], 1000h
   segprefixes 2
   cmp word [rsi+20h], 2000h
   segprefixes 2
   cmp word [rsi+30h], 3000h
   nop8
   %endmacro
   
%elif tcase == 18      ; case 18: cmp [basep+o8], i32;  L-L-L-S

   %macro testcode 0
   segprefixes 1
   cmp dword [rsi+10h], 100000h
   segprefixes 1
   cmp dword [rsi+20h], 200000h
   segprefixes 1
   cmp dword [rsi+30h], 300000h
   nop8
   %endmacro
   
%elif tcase == 19      ; case 19: cmp [basep+o32small], i8;  L-L-L-S

   %macro testinit2 0
   xor ebx,ebx
   %endmacro

   %macro testcode 0
   segprefixes 0
   cmp dword [rsi+rbx*1+7000h], 1
   segprefixes 0
   cmp dword [rsi+rbx*1+7010h], 2
   segprefixes 0
   cmp dword [rsi+rbx*1+7020h], 3
   nop8
   %endmacro
   
%elif tcase == 20      ; case 20: cmp [basep+o32small], i16;  L-L-L-S

   %macro testinit2 0
   xor ebx,ebx
   %endmacro

   %macro testcode 0
   segprefixes 0
   cmp word [rsi+rbx*2+7000h], 1000h
   segprefixes 0
   cmp word [rsi+rbx*2+7010h], 2000h
   segprefixes 0
   cmp word [rsi+rbx*2+7020h], 3000h
   nop2
   %endmacro
   
%elif tcase == 21      ; case 21: cmp [basep+o32small], i32small;  L-L-L-S

   %macro testcode 0
   cmp dword [rsi+7000h], 1000h
   cmp dword [rsi+7010h], 2000h
   cmp dword [rsi+7020h], 3000h
   nop2
   %endmacro
   
%elif tcase == 22      ; case 22: cmp [basep+o32small], i32;  L-L-L-S

   %macro testcode 0
   cmp dword [rsi+7000h], 10000000h
   cmp dword [rsi+7010h], 20000000h
   cmp dword [rsi+7020h], 30000000h
   nop2
   %endmacro
   
%elif tcase == 23      ; case 23: cmp [basep+o32], i8;  L-L-L-S

   %macro testcode 0
   segprefixes 1
   cmp dword [rsi+8000h], 1
   segprefixes 1
   cmp dword [rsi+8010h], 2
   segprefixes 1
   cmp dword [rsi+8020h], 3
   nop8
   %endmacro
   
%elif tcase == 24      ; case 24: cmp [basep+o32], i16;  L-L-L-S

   %macro testcode 0
   segprefixes 1
   cmp word [rsi+8000h], 1000h
   segprefixes 1
   cmp word [rsi+8010h], 2000h
   segprefixes 1
   cmp word [rsi+8020h], 3000h
   nop2
   %endmacro
   
%elif tcase == 25      ; case 25: cmp [basep+o32], i32;  L-L-L-S

   %macro testcode 0
   cmp dword [rsi+8000h], 10000000h
   cmp dword [rsi+8010h], 20000000h
   cmp dword [rsi+8020h], 30000000h
   nop2
   %endmacro
   
%elif tcase == 26      ; case 26: cmp [basep+s*index], reg;  L-L-L-S
   
   %macro testinit2 0
   xor ebx,ebx
   %endmacro

   %macro testcode 0
   segprefixes 4
   cmp dword [rsi+rbx*4], eax
   segprefixes 4
   cmp dword [rdi+rbx*4], ecx
   segprefixes 4
   cmp dword [rsi+rbx*4], edx
   nop11
   %endmacro
   
%elif tcase == 27      ; case 27: cmp [basep+s*index], i8;  L-L-L-S
   
   %macro testinit2 0
   xor ebx,ebx
   %endmacro

   %macro testcode 0
   segprefixes 4
   cmp dword [rsi+rbx*4], 1
   segprefixes 4
   cmp dword [rdi+rbx*4], 2
   segprefixes 4
   cmp dword [rsi+rbx*4], 3
   nop8
   %endmacro
   
%elif tcase == 28      ; case 28: cmp [basep+s*index], i16;  L-L-L-S
   
   %macro testinit2 0
   xor ebx,ebx
   %endmacro

   %macro testcode 0
   segprefixes 2
   cmp word [rsi+rbx*4], 1000h
   segprefixes 2
   cmp word [rdi+rbx*4], 2000h
   segprefixes 2
   cmp word [rsi+rbx*4], 3000h
   nop8
   %endmacro
   
%elif tcase == 29      ; case 29: cmp [basep+s*index], i32;  L-L-L-S
   
   %macro testinit2 0
   xor ebx,ebx
   %endmacro

   %macro testcode 0
   segprefixes 1
   cmp dword [rsi+rbx*4], 100000h
   segprefixes 1
   cmp dword [rdi+rbx*4], 200000h
   segprefixes 1
   cmp dword [rsi+rbx*4], 300000h
   nop8
   %endmacro
   
%elif tcase == 30      ; case 30: cmp [basep+s*index+o8], reg;  L-L-L-S
   
   %macro testinit2 0
   xor ebx,ebx
   %endmacro

   %macro testcode 0
   segprefixes 4
   cmp  [rsi+rbx*4+10h], eax
   segprefixes 4
   cmp  [rsi+rbx*4+14h], ecx
   segprefixes 4
   cmp  [rsi+rbx*4+18h], edx
   nop8
   %endmacro
   
%elif tcase == 31      ; case 31: cmp [basep+s*index+o8], i8;  L-L-L-S
   
   %macro testinit2 0
   xor ebx,ebx
   %endmacro

   %macro testcode 0
   segprefixes 3
   cmp dword [rsi+rbx*4+10h], 1
   segprefixes 3
   cmp dword [rsi+rbx*4+14h], 2
   segprefixes 3
   cmp dword [rsi+rbx*4+18h], 3
   nop8
   %endmacro
   
%elif tcase == 32      ; case 32: cmp [basep+s*index+o8], i16;  L-L-L-S
   
   %macro testinit2 0
   xor ebx,ebx
   %endmacro

   %macro testcode 0
   segprefixes 1
   cmp word [rsi+rbx*4+10h], 1000h
   segprefixes 1
   cmp word [rsi+rbx*4+14h], 2000h
   segprefixes 1
   cmp word [rsi+rbx*4+18h], 3000h
   nop8
   %endmacro
   
%elif tcase == 33      ; case 31: cmp [basep+s*index+o8], i32;  L-L-L-S
   
   %macro testinit2 0
   xor ebx,ebx
   %endmacro

   %macro testcode 0
   segprefixes 1
   cmp dword [rsi+rbx*4+10h], 100000h
   segprefixes 1
   cmp dword [rsi+rbx*4+14h], 200000h
   segprefixes 1
   cmp dword [rsi+rbx*4+18h], 300000h
   nop5
   %endmacro
   
%elif tcase == 34      ; case 34: cmp [basep+s*index+o32small], reg;  L-L-L-S
   
   %macro testinit2 0
   xor ebx,ebx
   %endmacro

   %macro testcode 0
   segprefixes 1
   cmp  [rsi+rbx*4+1000h], eax
   segprefixes 1
   cmp  [rsi+rbx*4+1004h], ecx
   segprefixes 1
   cmp  [rsi+rbx*4+1008h], edx
   nop8
   %endmacro
   
%elif tcase == 35      ; case 35: cmp [basep+s*index+o32small], i8;  L-L-L-S
   
   %macro testinit2 0
   xor ebx,ebx
   %endmacro

   %macro testcode 0
   segprefixes 0
   cmp dword [rsi+rbx*4+1000h], 1
   segprefixes 0
   cmp dword [rsi+rbx*4+1004h], 2
   segprefixes 0
   cmp dword [rsi+rbx*4+1008h], 3
   nop8
   %endmacro
   
%elif tcase == 36      ; case 36: cmp [basep+s*index+o32small], i16;  L-L-L-S
   
   %macro testinit2 0
   xor ebx,ebx
   %endmacro

   %macro testcode 0
   cmp word [rsi+rbx*4+1000h], 1000h
   cmp word [rsi+rbx*4+1004h], 2000h
   cmp word [rsi+rbx*4+1008h], 3000h
   nop2
   %endmacro
   
%elif tcase == 37      ; case 37: cmp [basep+s*index+o32small], i32;  L-S-S-S
   
   %macro testinit2 0
   xor ebx,ebx
   %endmacro

   %macro testcode 0
   cmp dword [rsi+rbx*4+1000h], 1000000h
   nop7
   nop7
   nop7
   %endmacro
   
%elif tcase == 38      ; case 38: cmp [rip], reg;  L-L-L-S

   %macro testcode 0
   cmp  [UserData + 10000h], rax
   cmp  [UserData + 10010h], rbx
   cmp  [UserData + 10020h], rcx
   nop11
   %endmacro
   
%elif tcase == 39      ; case 39: cmp [rip small], i8;  L-L-L-S

   %macro testcode 0
   cmp byte [TestLoop + 1000h], 1
   cmp byte [TestLoop + 1010h], 2
   cmp byte [TestLoop + 1020h], 3
   nop11
   %endmacro
   
%elif tcase == 40      ; case 40: cmp [rip small], i16;  L-L-L-S

   %macro testcode 0
   cmp word [TestLoop + 1000h], 100h
   cmp word [TestLoop + 1010h], 200h
   cmp word [TestLoop + 1020h], 300h
   nop5
   %endmacro
   
%elif tcase == 41      ; case 41: cmp [rip small], i32small;  L-L-L-S

   %macro testcode 0
   cmp dword [TestLoop + 1000h], 100h
   cmp dword [TestLoop + 1010h], 200h
   cmp dword [TestLoop + 1020h], 300h
   nop2
   %endmacro
   
%elif tcase == 42      ; case 42: cmp [rip small], i32;  L-L-L-S

   %macro testcode 0
   cmp dword [TestLoop + 1000h], 100000h
   cmp dword [TestLoop + 1010h], 200000h
   cmp dword [TestLoop + 1020h], 300000h
   nop2
   %endmacro

%elif tcase == 43      ; case 43: cmp [rip], i8;  L-L-L-S

   %macro testcode 0
   cmp byte [UserData + 10000h], 1
   cmp byte [UserData + 10010h], 2
   cmp byte [UserData + 10020h], 3
   nop11
   %endmacro
   
%elif tcase == 44      ; case 44: cmp [rip], i16;  L-L-L-S

   %macro testcode 0
   cmp word [UserData + 10000h], 100h
   cmp word [UserData + 10010h], 200h
   cmp word [UserData + 10020h], 300h
   nop5
   %endmacro
   
%elif tcase == 45      ; case 45: cmp [rip], i32small;  L-L-L-S

   %macro testcode 0
   cmp dword [UserData + 10000h], 100h
   cmp dword [UserData + 10010h], 200h
   cmp dword [UserData + 10020h], 300h
   nop2
   %endmacro
   
%elif tcase == 46      ; case 46: cmp [rip], i32;  L-L-L-S

   %macro testcode 0
   cmp dword [UserData + 10000h], 100000h
   cmp dword [UserData + 10010h], 200000h
   cmp dword [UserData + 10020h], 300000h
   nop2
   %endmacro
   
%elif tcase == 47      ; case 47: mov [basep], i32small;  L-S-S-S

   %macro testcode 0
   mov dword [rsi], 1000h
   nop9
   nop9
   nop8
   %endmacro
   
%elif tcase == 48      ; case 48: mov [basep], i32;  L-S-S-S

   %macro testcode 0
   mov dword [rsi], 100000h
   nop9
   nop9
   nop8
   %endmacro
   
%elif tcase == 49      ; case 49: mov [basep+s*index], i32small;  L-S-S-S

   %macro testinit2 0
   xor ebx,ebx
   %endmacro

   %macro testcode 0
   mov dword [rsi+rbx*4], 1000h
   nop9
   nop8
   nop8
   %endmacro
   
%elif tcase == 50      ; case 50: mov [basep+s*index], i32;  L-S-S-S

   %macro testinit2 0
   xor ebx,ebx
   %endmacro

   %macro testcode 0
   mov dword [rsi+rbx*4], 100000h
   nop9
   nop8
   nop8
   %endmacro
   
%elif tcase == 51      ; case 51: cmp [rip], 0;  L-L-L-S

   %macro testcode 0
   cmp byte [UserData], 0
   cmp byte [UserData+4], 0
   cmp byte [UserData+8], 0
   nop11
   %endmacro
   
%elif tcase == 52      ; case 52: cmp [rip], i8;  L-S-S-S

   %macro testcode 0
   cmp byte [UserData], 100
   nop9
   nop8
   nop8
   %endmacro
   
%elif tcase == 53      ; case 53: cmp [rip], i16;  L-S-S-S

   %macro testcode 0
   cmp word [UserData], 1000h
   nop7
   nop8
   nop8
   %endmacro
   
%elif tcase == 54      ; case 54: cmp [rip], i32;  L-S-S-S

   %macro testcode 0
   cmp dword [UserData], 10000000h
   nop6
   nop8
   nop8
   %endmacro       

%elif tcase == 55      ; case 55: cmp FS:[o32small], i8;  L-S-S-S

   %macro testcode 0
   cmp dword [fs:8],1
   nop7
   nop8
   nop8
   %endmacro
   
%elif tcase == 56      ; case 56: cmp FS:[o32small], i8;  L-L-L-S

   %macro testcode 0
   cmp dword [fs:4],1
   cmp dword [fs:8],1
   cmp dword [fs:12],1
   nop5
   %endmacro
   
%elif tcase == 57      ; case 57: cmp FS:[o32small], i16;  L-L-S-S

   %macro testcode 0
   cmp word [fs:4],1000h
   cmp word [fs:8],2000h
   nop5
   nop5
   %endmacro
   
%elif tcase == 58      ; case 58: cmp FS:[o32small], i32small;  L-L-S-S

   %macro testcode 0
   cmp dword [fs:4],1000h
   cmp dword [fs:8],2000h
   nop4
   nop4
   %endmacro
   
%elif tcase == 59      ; case 59: cmp FS:[o32small], i32;  L-L-S-S

   %macro testcode 0
   cmp dword [fs:4],100000h
   cmp dword [fs:8],200000h
   nop4
   nop4
   %endmacro
   
%elif tcase == 60      ; case 60: roundps xmm,[rip], i8;  L-S-S-S

   %macro testcode 0
   roundps xmm0,[UserData],1
   nop6
   nop8
   nop8
   %endmacro
   
%elif tcase == 61      ; case 61: roundps xmm,[rip], i8;  L-L-L-S

   %macro testcode 0
   roundps xmm0,[UserData],1
   roundps xmm1,[UserData+10h],1
   roundps xmm2,[UserData+20h],1
   nop2
   %endmacro

%elif tcase == 62      ; case 62: vinsertf128 y,y,[rip],1;  L-S-S-S

   %macro testcode 0
   vinsertf128 ymm0,ymm8,[UserData],1
   nop6
   nop8
   nop8
   %endmacro
   
%elif tcase == 63      ; case 63: vinsertf128 y,y,[rip],1;  L-L-L-S

   %macro testcode 0
   vinsertf128 ymm0,ymm8,[UserData],1
   vinsertf128 ymm1,ymm8,[UserData+10H],1
   vinsertf128 ymm2,ymm8,[UserData+20H],1
   nop2
   %endmacro

%elif tcase == 64      ; case 64: mov r,[abs64];  L-S-S-S

   %macro testcode 0
   mov eax,[abs qword UserData]
   nop9
   nop9
   nop5
   %endmacro

%elif tcase == 65      ; case 65: cmp [basep+s*index+o32], reg;  L-L-L-S
   
   %macro testinit2 0
   xor ebx,ebx
   lea rdi, [rsi-40000000h]
   %endmacro

   %macro testcode 0
   segprefixes 1
   cmp  [rdi+rbx*4+40000000h], eax
   segprefixes 1
   cmp  [rdi+rbx*4+40000004h], ecx
   segprefixes 1
   cmp  [rdi+rbx*4+40000008h], edx
   nop8
   %endmacro

%elif tcase == 66      ; case 66: cmp [basep+s*index+o32], i8;  L-L-L-S
   
   %macro testinit2 0
   xor ebx,ebx
   lea rdi, [rsi-40000000h]
   %endmacro

   %macro testcode 0
   segprefixes 0
   cmp  byte [rdi+rbx*4+40000000h], 1
   segprefixes 0
   cmp  byte [rdi+rbx*4+40000004h], 2
   segprefixes 0
   cmp  byte [rdi+rbx*4+40000008h], 3
   nop8
   %endmacro

%elif tcase == 67      ; case 67: mov [basep+o32], r;  L-L-L-S
   
   %macro testinit2 0
   xor ebx,ebx
   lea rdi, [rsi-40000000h]
   %endmacro

   %macro testcode 0
   segprefixes 1
   mov  dword [rdi+40000000h], eax
   segprefixes 1
   mov  dword [rdi+40000004h], eax
   segprefixes 1
   mov  dword [rdi+40000008h], eax
   nop11
   %endmacro

%elif tcase == 68      ; case 68: mov [basep+s*index+o32], r;  L-L-L-S
   
   %macro testinit2 0
   xor ebx,ebx
   lea rdi, [rsi-40000000h]
   %endmacro

   %macro testcode 0
   segprefixes 1
   mov  dword [rdi+rbx*4+40000000h], eax
   segprefixes 1
   mov  dword [rdi+rbx*4+40000004h], eax
   segprefixes 1
   mov  dword [rdi+rbx*4+40000008h], eax
   nop8
   %endmacro

%elif tcase == 69      ; case 69: cmp r,r / jc o32small;  L-L-S-S
   
   %macro testinit2 0
   xor ebx,ebx
   lea r8d,[rbx+1]
   %endmacro

   %macro testcode 0
   segprefixes 1
   cmp rbx,r8
   je $+1000h
   nop11
   nop11
   %endmacro

%elif tcase == 70      ; case 70: cmp [basep+o32], r / jc o32small;  L-L-S-S
   
   %macro testinit2 0
   mov ebx,12345
   lea rdi, [rsi-40000000h]
   %endmacro

   %macro testcode 0
   segprefixes 1
   cmp rbx,[rdi+40000000h]
   je $+1000h
   nop9
   nop9
   %endmacro

%elif tcase == 71      ; case 71: cmp [r+s*r], r / jc o32small;  L-L-S-S
   
   %macro testinit2 0
   xor ebx,ebx
   mov r8d,12345
   %endmacro

   %macro testcode 0
   segprefixes 1
   cmp [rsi+4*rbx],r8
   je $+1000h
   nop10
   nop11
   %endmacro

%elif tcase == 72      ; case 72: cmp [r+s*r+o32small], r / jc o32small;  L-L-S-S
   
   %macro testinit2 0
   xor ebx,ebx
   mov r8d,12345
   %endmacro

   %macro testcode 0
   segprefixes 1
   cmp [rsi+4*rbx+1000h],r8
   je $+1000h
   nop10
   nop7
   %endmacro

%elif tcase == 73      ; case 73: fma r,r,[r];  L-L-L-S
   
   %macro testinit2 0
   xor ebx,ebx
   lea rdi,[rsi+10h]
   %endmacro

   %macro testcode 0
   segprefixes 2   
   vfmadd231pd xmm1,xmm8,[rsi]
   segprefixes 2   
   vfnmadd132pd xmm2,xmm9,[rdi]
   segprefixes 2   
   vfmadd231pd xmm3,xmm10,[rsi]
   nop11
   %endmacro

%elif tcase == 74      ; case 74: fma r,r,[r+s*r];  L-L-L-S
   
   %macro testinit2 0
   xor ebx,ebx
   lea rdi,[rsi+10h]
   %endmacro

   %macro testcode 0
   segprefixes 1   
   vfmadd231pd xmm1,xmm8,[rsi+4*rbx]
   segprefixes 1   
   vfnmadd132pd xmm2,xmm9,[rdi+8*rbx]
   segprefixes 1   
   vfmadd231pd xmm3,xmm10,[rsi+4*rbx]
   nop11
   %endmacro

%elif tcase == 75      ; case 75: fma r,r,[r+s*r+o32small];  L-L-L-S
   
   %macro testinit2 0
   xor ebx,ebx
   lea rdi,[rsi+10h]
   %endmacro

   %macro testcode 0
   segprefixes 0
   vfmadd231pd  xmm0,xmm8, [rsi+4*rbx+1000h]
   segprefixes 0   
   vfnmadd132pd xmm1,xmm9, [rdi+8*rbx+1020h]
   vfmadd231pd  xmm2,xmm10,[rsi+4*rbx+1040h]
   nop2
   %endmacro
   
%elif tcase == 76      ; case 76: fma r,r,[r+s*r+o32];  L-L-L-S
   
   %macro testinit2 0
   xor ebx,ebx
   lea rdi, [rsi-40000000h]
   %endmacro

   %macro testcode 0
   segprefixes 0
   vfmadd231pd  xmm0,xmm8, [rdi+4*rbx+40000000h]
   segprefixes 0   
   vfnmadd132pd xmm1,xmm9, [rdi+8*rbx+40000020h]
   vfmadd231pd  xmm2,xmm10,[rdi+4*rbx+40000040h]
   nop2
   %endmacro
   
%else 

   %error unknown test case tcase

%endif
