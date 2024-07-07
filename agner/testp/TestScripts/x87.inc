; x87.inc
; Define test code for x87 type floating point instructions
; (c) Copyright 2012 by Agner Fog. GNU General Public License www.gnu.org/licenses

; Parameters:
;
; instruct:   Instruction to test
;
; tmode:      Test mode:
;             L:  measure latency
;             T:  measure throughput
;             M:  measure throughput with memory source operand
;             (other values instruction specific)
;
; tcase:      best:  trivial data for fastest case, 
;             worst: non-trivial data for slow case
;
; msize:      size of memory operand: 16, 32, 64, 80
;
; dividend:   dividend in division
;
; divisor:    divisor in division

; Default values:

%ifndef repeat1
   %define repeat1 100
%endif

%ifndef tmode
   %define tmode  L
%endif

%ifndef tcase
   %define tcase  worst
%endif

%ifndef msize
   %define msize  32
%endif

%ifndef dividend
   %define dividend 8.1509281715106E12   ; div1
%endif

%ifndef divisor
   %define divisor 1.20278165192619   ; div2
%endif

%macro testdata 0
   times 10000H  DB 0
   div1 dq dividend
   div2 dq divisor
%endmacro

; define memory size override
%if msize == 16
   %define mpointer word
%elif msize == 32
   %define mpointer dword
%elif msize == 64
   %define mpointer qword
%elif msize == 80
   %define mpointer tword
%else
   %error unknown msize
%endif

; Define specific test code for each instruction case:

%ifidni instruct, f2xm1

   %macro testinit2 0
      %ifidni tcase, best
         fldz
         fldz
      %else  ; worst case
         fld qword [div1]
         fld st0
         xor eax,eax
      %endif
   %endmacro
   %macro testafter2 0
      fstp st0
      fstp st0
   %endmacro
   %macro testcode 0
      %ifidni tcase, best
         f2xm1
      %else  ; worst case
         f2xm1
         fcmovnb st0,st1
      %endif
   %endmacro

%elifidni instruct, fabs
   %macro testinit2 0
      fldz
      fldz
   %endmacro
   %macro testafter2 0
      fstp st0
      fstp st0
   %endmacro
   %macro testcode 0
      %ifidni tmode, L
         fabs
      %else  ; throughput
         fabs
         fxch
      %endif
   %endmacro

%elifidni instruct, fadd
   %define repeat2 1
   %macro testinit2 0
      fldz
      fldz
      fldz
      fldz
      fldz
   %endmacro
   %macro testafter2 0
      fstp st0
      fstp st0
      fstp st0
      fstp st0
      fstp st0
   %endmacro
   %macro testcode 0
      %ifidni tmode, L    ; latency
         %rep 100
            fadd st0,st0
         %endrep
      %elifidni tmode, T  ; throughput
         %rep 25
            fadd st1,st0
            fadd st2,st0
            fadd st3,st0
            fadd st4,st0
         %endrep
      %elifidni tmode, M  ; throughput with memory operand
         %rep 50
            fadd mpointer [rsi]
            fxch st1
            fadd mpointer [rsi+8]
            fxch st2
         %endrep
      %endif
   %endmacro

%elifidni instruct, fbld

   %macro testcode 0
      %ifidni tmode, T   ; throughput
         fbld tword [rsi]
         fstp st0
      %elifidni tmode, L    ; latency
         fbld tword [rsi]
         fstp tword [rsi]
      %endif
   %endmacro

%elifidni instruct, fchs
   %macro testinit2 0
      fldz
      fldz
   %endmacro
   %macro testafter2 0
      fstp st0
      fstp st0
   %endmacro
   %macro testcode 0
      %ifidni tmode, L
         fchs
      %else  ; throughput
         fchs
         fxch
      %endif
   %endmacro

%elifidni instruct, fcmov

   %macro testinit2 0
      fldz
      fldz
      fldz
   %endmacro
   %macro testafter2 0
      fstp st0
      fstp st0
      fstp st0
   %endmacro
   %macro testcode 0
      %ifidni tmode, L      ; latency
         fcmove st0,st0
      %elifidni tmode, T    ; throughput
         fcmove st0,st2
         fxch st0,st1
      %endif
   %endmacro

%elifidni instruct, fcomi

   %macro testinit2 0
      fldz
      fldz
   %endmacro
   %macro testafter2 0
      fstp st0
      fstp st0
   %endmacro
   %macro testcode 0
      %ifidni tmode, L      ; latency
         fcomi st1
         fcmove st0,st1
      %elifidni tmode, T    ; throughput
         fcomi st1
      %endif
   %endmacro

%elifidni instruct, fcom

   %macro testinit2 0
      fldz
      fldz
   %endmacro
   %macro testafter2 0
      fstp st0
      fstp st0
   %endmacro
   %macro testcode 0
      %ifidni tmode, L      ; latency
         fcom st1
         fnstsw ax
         test ah, 1
         fcmove st0,st1
      %elifidni tmode, T    ; throughput
         fcom st1
      %elifidni tmode, M    ; throughput with memory operand
         fcom mpointer [rsi]
      %endif
   %endmacro

%elifidni instruct, fcompp

   %macro testinit2 0
      fldz
      fldz
   %endmacro
   %macro testafter2 0
      fstp st0
      fstp st0
   %endmacro
   %macro testcode 0
      %ifidni tmode, L      ; latency
         fld st1
         fld st1
         fcompp
         fnstsw ax
         test ah, 1
         fcmove st0,st1
      %elifidni tmode, T    ; throughput
         fld st1
         fld st1
         fcompp
      %endif
   %endmacro

%elifidni instruct, fcos

   %macro testinit2 0
      %ifidni tcase, best
         fldz
         fldz
         xor eax,eax
     %else
         fldz
         fld qword [div1]
     %endif
   %endmacro
   %macro testafter2 0
      fstp st0
      fstp st0
   %endmacro
   %macro testcode 0
      %ifidni tcase, best
         fcos
         fcmovnb st0,st1
      %else   ; worst case
         fcos
      %endif
   %endmacro

%elifidni instruct, fdiv

   %define repeat2 1         ; disable default macro loop
   %macro testinit2 0
      %ifidni tcase, best
         fld1
         fld1
         fld1
         fld1
         fld1
         fld1
     %else                   ; worst case
         fld qword [div1]    ; dividend = high number
         fld qword [div1]    ; dividend = high number
         fld qword [div1]
         fld qword [div1]
         fld qword [div1]
         fld qword [div2]    ; divisor = close to 1
     %endif
     fst mpointer [rsi]      ; save divisor
   %endmacro
   %macro testafter2 0
      fcompp                 ; clean up f.p. stack
      fcompp
      fcompp
   %endmacro
   %macro testcode 0
      %ifidni tmode, L  ; latency
         %rep 100
            fdiv st1,st0
         %endrep
         fxch st5           ; avoid dividend becoming too small
         fadd st1,st0
         fxch st5
      %elifidni tmode, T  ; throughput
         %rep 25
            fdiv st1,st0
            fdiv st2,st0
            fdiv st3,st0
            fdiv st4,st0
         %endrep
         fxch st5           ; avoid dividend becoming too small
         fadd st1,st0 
         fadd st2,st0
         fadd st3,st0
         fadd st4,st0
         fxch st5
      %elifidni tmode, M  ; throughput with memory operand
         %rep 50
            fdiv mpointer [rsi]
            fxch st0,st1
            fdiv mpointer [rsi]
            fxch st0,st2
         %endrep
         fld st5           ; avoid dividend becoming too small
         fadd st1,st0 
         fadd st2,st0
         fadd st3,st0
         fstp st0
      %endif
   %endmacro

%elifidni instruct, ffree

   %macro testcode 0
      ffree st1
   %endmacro

%elifidni instruct, fiadd

   %define repeat2 1         ; disable default macro loop
   %macro testinit2 0
      fldz
      fldz
      fldz
      fldz
      fldz
      fist mpointer [rsi]
   %endmacro
   %macro testafter2 0
      fstp st0
      fstp st0
      fstp st0
      fstp st0
      fstp st0
   %endmacro
   %macro testcode 0
      %ifidni tmode, L  ; latency
         %rep 100
            fiadd mpointer [rsi]
         %endrep
      %else             ; T, M: throughput
         %rep 25
            fiadd mpointer [rsi]
            fxch st0,st1
            fiadd mpointer [rsi]
            fxch st0,st2
            fiadd mpointer [rsi]
            fxch st0,st3
            fiadd mpointer [rsi]
            fxch st0,st4         
         %endrep
      %endif
   %endmacro

%elifidni instruct, ficom

   %macro testinit2 0
      fld1
   %endmacro
   %macro testafter2 0
      fstp st0
   %endmacro
   %macro testcode 0
      ficom mpointer [rsi]
   %endmacro

%elifidni instruct, fidiv    ; best case only

   %define repeat2 1         ; disable default macro loop
   %macro testinit2 0
      fld1
      fld1
      fld1
      fld1
      fld1
      fist mpointer [rsi]
   %endmacro
   %macro testafter2 0
      fstp st0
      fstp st0
      fstp st0
      fstp st0
      fstp st0
   %endmacro
   %macro testcode 0
      %ifidni tmode, L  ; latency
         %rep 100
            fidiv mpointer [rsi]
         %endrep
      %else             ; T, M: throughput
         %rep 25
            fidiv mpointer [rsi]
            fxch st0,st1
            fidiv mpointer [rsi]
            fxch st0,st2
            fidiv mpointer [rsi]
            fxch st0,st3
            fidiv mpointer [rsi]
            fxch st0,st4         
         %endrep
      %endif
   %endmacro

%elifidni instruct, fild
   %macro testcode 0
      %ifidni tmode, T  ; throughput
         fild mpointer [rsi]
         fstp st0
      %elifidni tmode, L  ; latency + fst
         fild mpointer [rsi]
         fstp mpointer [rsi]
      %elifidni tmode, LL  ; latency + fist
         fild mpointer [rsi]
         fistp mpointer [rsi]
      %endif
   %endmacro

%elifidni instruct, fimul

   %define repeat2 1         ; disable default macro loop
   %macro testinit2 0
      fld1
      fld1
      fld1
      fld1
      fld1
      fist mpointer [rsi]
   %endmacro
   %macro testafter2 0
      fstp st0
      fstp st0
      fstp st0
      fstp st0
      fstp st0
   %endmacro
   %macro testcode 0
      %ifidni tmode, L  ; latency
         %rep 100
            fimul mpointer [rsi]
         %endrep
      %else             ; T, M: throughput
         %rep 25
            fimul mpointer [rsi]
            fxch st0,st1
            fimul mpointer [rsi]
            fxch st0,st2
            fimul mpointer [rsi]
            fxch st0,st3
            fimul mpointer [rsi]
            fxch st0,st4         
         %endrep
      %endif
   %endmacro

%elifidni instruct, fist

   %macro testinit2 0
      fld1
   %endmacro
   %macro testafter2 0
      fstp st0
   %endmacro
   %macro testcode 0
      %ifidni tmode, T  ; throughput
         fist mpointer [rsi]
      %elifidni tmode, L  ; latency + fld
         fld mpointer [rsi]
         fistp mpointer [rsi]
      %elifidni tmode, LL  ; latency + fild
         fild mpointer [rsi]
         fistp mpointer [rsi]
      %endif
   %endmacro
   
%elifidni instruct, fisttp

   %macro testinit2 0
      fld1
   %endmacro
   %macro testafter2 0
      fstp st0
   %endmacro
   %macro testcode 0
      %ifidni tmode, T  ; throughput
         fld st0
         fisttp mpointer [rsi]
      %elifidni tmode, L  ; latency + fld
         fld mpointer [rsi]
         fisttp mpointer [rsi]
      %elifidni tmode, LL  ; latency + fild
         fild mpointer [rsi]
         fisttp mpointer [rsi]
      %endif
   %endmacro   

%elifidni instruct, fld

   %macro testinit2 0
      fld1
   %endmacro
   %macro testafter2 0
      fstp st0
   %endmacro
   %macro testcode 0
      %ifidni tmode,T      ; throughput
         fld st0
         fstp st0
      %elifidni tmode,L    ; latency
         fld st0
         fstp st1
      %elifidni tmode,M    ; throughput with memory operand
         fld mpointer [rsi]
         fstp st0
      %elifidni tmode,LM    ; latency with memory operand
         fld mpointer [rsi]
         fstp mpointer [rsi]
      %endif
   %endmacro

%elifidni instruct, fst

   %macro testinit2 0
      fld1
      fld1
   %endmacro
   %macro testafter2 0
      fstp st0
      fstp st0
   %endmacro
   %macro testcode 0
      %ifidni tmode, T      ; throughput
         fst st1
      %elifidni tmode, L    ; latency
         fst st0
      %elifidni tmode, M    ; throughput with memory
         fst mpointer [rsi]
      %endif
   %endmacro

%elifidni instruct, fstp

   %macro testinit2 0
      fld1
      fld1
   %endmacro
   %macro testafter2 0
      fstp st0
      fstp st0
   %endmacro
   %macro testcode 0
      %ifidni tmode, T      ; throughput
         fld st0
         fstp st0
      %elifidni tmode, L    ; latency
         fld st0
         fstp st1
      %elifidni tmode, M    ; throughput with memory
         fld st0
         fstp mpointer [rsi]
      %endif
   %endmacro

%elifidni instruct, fld1

   %macro testcode 0
      fld1
      fstp st0
   %endmacro

%elifidni instruct, fldz

   %macro testcode 0
      fldz
      fstp st0
   %endmacro

%elifidni instruct, fldpi

   %macro testcode 0
      fldpi
      fstp st0
   %endmacro

%elifidni instruct, fldcw

   %define repeat2 1         ; disable default macro loop
   %macro testinit2 0
      fld1
      fnstcw word [rsi]
      movzx eax, word [rsi]
      xor   eax, 0c00h
      fnstcw word [rsi+4]
   %endmacro
   %macro testafter2 0
      fldcw word [rsi]
      fstp st0
   %endmacro
   %macro testcode 0
      %ifidni tmode, T  ; throughput, same value
         %rep 100
            fldcw word [rsi]
         %endrep
      %elifidni tmode, TT  ; throughput, alternating value
         %rep 50
            fldcw word [rsi]
            fldcw word [rsi+4]
         %endrep
      %elifidni tmode, L  ; latency + fnstcw
         %rep 100
            fldcw word [rsi]
            fnstcw word [rsi]
         %endrep
      %endif
   %endmacro

%elifidni instruct, fnstcw

   %define repeat2 1         ; disable default macro loop
   %macro testinit2 0
      fld1
      fnstcw word [rsi]
   %endmacro
   %macro testafter2 0
      fstp st0
   %endmacro
   %macro testcode 0
      %ifidni tmode, T  ; throughput, same value
         %rep 100
            fnstcw word [rsi]
         %endrep
      %elifidni tmode, L  ; latency + fldcw
         %rep 100
            fldcw word [rsi]
            fnstcw word [rsi]
         %endrep
      %endif
   %endmacro

%elifidni instruct, fbstp

   %macro testinit2 0
      fldpi
   %endmacro
   %macro testafter2 0
      fstp st0
   %endmacro
   %macro testcode 0
      %ifidni tmode, T  ; throughput
         fld st0
         fbstp tword [rsi]
      %elifidni tmode, L  ; latency + fld qword
         fld qword [rsi]
         fbstp tword [rsi]
      %elifidni tmode, LL  ; latency + fld tbyte
         fld tword [rsi]
         fbstp tword [rsi]
      %elifidni tmode, LLL  ; latency + fbld tbyte
         fbld tword [rsi]
         fbstp tword [rsi]
      %endif
   %endmacro

%elifidni instruct, fistp

   %macro testinit2 0
      fldpi
   %endmacro
   %macro testafter2 0
      fstp st0
   %endmacro
   %macro testcode 0
      %ifidni tmode, T  ; throughput
         fld st0
         fistp mpointer [rsi]
      %elifidni tmode, L  ; latency + fld m
         fld mpointer [rsi]
         fistp mpointer [rsi]
      %elifidni tmode, LL  ; latency + fild m
         fild mpointer [rsi]
         fistp mpointer [rsi]
      %endif
   %endmacro

%elifidni instruct, fstp

   %macro testinit2 0
      fldpi
   %endmacro
   %macro testafter2 0
      fstp st0
   %endmacro
   %macro testcode 0
      %ifidni tmode, T  ; throughput
         fld st0
         fstp mpointer [rsi]
      %elifidni tmode, L  ; latency + fld m
         fld mpointer [rsi]
         fstp mpointer [rsi]
      %elifidni tmode, LL  ; latency + fld m + fchs
         fld mpointer [rsi]
         fchs
         fstp mpointer [rsi]
      %endif
   %endmacro

%elifidni instruct, fmul
   %define repeat2 1
   %macro testinit2 0
      fld1
      fld1
      fld1
      fld1
      fld1
   %endmacro
   %macro testafter2 0
      fstp st0
      fstp st0
      fstp st0
      fstp st0
      fstp st0
   %endmacro
   %macro testcode 0
      %ifidni tmode, L    ; latency
         %rep 100
            fmul st0,st0
         %endrep
      %elifidni tmode, T  ; throughput
         %rep 25
            fmul st1,st0
            fmul st2,st0
            fmul st3,st0
            fmul st4,st0
         %endrep
      %elifidni tmode, M  ; throughput with memory operand
         %rep 50
            fmul mpointer [rsi]
            fxch st1
            fmul mpointer [rsi+8]
            fxch st2
         %endrep
      %endif
   %endmacro

%elifidni instruct, fnsave

   %macro testinit2 0
      fld1
      fld1
   %endmacro
   %macro testafter2 0
      fstp st0
      fstp st0
   %endmacro
   %macro testcode 0
      %ifidni tmode, T  ; throughput
         fnsave [psi] 
      %elifidni tmode, L  ; latency + frstor
         fnsave [psi]
         frstor [psi]
      %endif
   %endmacro

%elifidni instruct, fxsave

   %macro testinit2 0
      fldpi
   %endmacro
   %macro testafter2 0
      fstp st0
   %endmacro
   %macro testcode 0
      %ifidni tmode, T  ; throughput
         fxsave [psi]
      %elifidni tmode, L  ; latency + fxrstor
         fxsave [psi]
         fxrstor [psi]
      %endif
   %endmacro

%elifidni instruct, frstor

   %macro testinit2 0
      fldpi
      fnsave [psi]
   %endmacro
   %macro testafter2 0
      fstp st0
   %endmacro
   %macro testcode 0
      %ifidni tmode, T  ; throughput
         frstor [psi]
      %elifidni tmode, L  ; latency + fnsave
         fnsave [psi]
         frstor [psi]
      %endif
   %endmacro

%elifidni instruct, fxrstor

   %macro testinit2 0
      fldpi
      fxsave [psi]
   %endmacro
   %macro testafter2 0
      fstp st0
   %endmacro
   %macro testcode 0
      %ifidni tmode, T  ; throughput
         fxrstor [psi]
      %elifidni tmode, L  ; latency + fnsave
         fxsave [psi]
         fxrstor [psi]
      %endif
   %endmacro


%elifidni instruct, xsave

   %macro testinit2 0
      fldpi
      lea psi, [UserData+40h]
      and psi, -40h               ; align by 64
      or eax, -1                  ; mask
      mov edx, eax
   %endmacro
   %macro testafter2 0
      fstp st0
   %endmacro
   %macro testcode 0
      %ifidni tmode, T  ; throughput
         instruct [psi]
      %elifidni tmode, L  ; latency + xrstor
         xsave [psi]
         xrstor [psi]
      %endif
   %endmacro

%elifidni instruct, xsaveopt

   %macro testinit2 0
      fldpi
      lea psi, [UserData+40h]
      and psi, -40h               ; align by 64
      or eax, -1                  ; mask
      mov edx, eax
      xsave [psi]
      xrstor [psi]
   %endmacro
   %macro testafter2 0
      fstp st0
   %endmacro
   %macro testcode 0
      %ifidni tmode, T  ; throughput
         instruct [psi]
      %elifidni tmode, L  ; latency + xrstor
         xsaveopt [psi]
         xrstor [psi]
      %endif
   %endmacro

%elifidni instruct, xrstor

   %macro testinit2 0
      fldpi
      lea psi, [UserData+40h]
      and psi, -40h               ; align by 64
      or eax, -1                  ; mask
      mov edx, eax
      xsave [psi]
   %endmacro
   %macro testafter2 0
      fstp st0
   %endmacro
   %macro testcode 0
      %ifidni tmode, T  ; throughput
         instruct [psi]
      %elifidni tmode, L  ; latency + xrstor
         xsave  [psi]
         xrstor [psi]
      %endif
   %endmacro

%elifidni instruct, fnstsw

   %macro testinit2 0
      fldpi
      fld1
      fnstsw [psi]
      mov    ebx, 100h
   %endmacro
   %macro testafter2 0
      fstp st0
      fstp st0
   %endmacro
   %macro testcode 0
      %ifidni tmode, T  ; throughput
         fnstsw ax
      %elifidni tmode, M  ; throughput, with memory operand
         fnstsw word [psi]
      %elifidni tmode, L  ; latency + test + fcmov + fcom
         fnstsw ax
         test   ah, 1
         fcmove st0,st1
         fcom   st0,st1
      %elifidni tmode, LM  ; latency, with memory operand
         fnstsw word [psi]
         test   word [psi], bx
         fcmove st0,st1
         fcom   st0,st1
      %endif
   %endmacro

%elifidni instruct, fpatan

   %macro testinit2 0
      %ifidni tcase, best
         fldz
         fldz
         fld1
      %else
         fld  qword [div1]
         fld st0
         fld  qword [div2]
      %endif
   %endmacro
   %macro testafter2 0
      fstp st0
      fstp st0
      fstp st0
   %endmacro
   %macro testcode 0
      %ifidni tmode, T  ; throughput
         fld st1
         fld st1
         fpatan
         fstp st0
      %elifidni tmode, L  ; latency + fld + fxch
         fpatan
         fld st1
         fxch
      %endif
   %endmacro


%elifidni instruct, fprem

   %macro testinit2 0
      %ifidni tcase, best
         fld1
         fld1
         fld1
      %else
         fld qword [div1]    ; dividend
         fld qword [div2]    ; divisor
         fld qword [div1]    ; dividend
      %endif
   %endmacro
   %macro testafter2 0
      fstp st0
      fstp st0
      fstp st0
   %endmacro
   %macro testcode 0
      %ifidni tmode, T  ; throughput + fld + fstp
         fprem
         fstp st0
         fld st1
      %elifidni tmode, L  ; latency + fadd
         fprem
         fadd st0,st2
      %endif
   %endmacro

%elifidni instruct, fprem1

   %macro testinit2 0
      %ifidni tcase, best
         fld1
         fld1
         fld1
      %else
         fld qword [div1]    ; dividend
         fld qword [div2]    ; divisor
         fld qword [div1]    ; dividend
      %endif
   %endmacro
   %macro testafter2 0
      fstp st0
      fstp st0
      fstp st0
   %endmacro
   %macro testcode 0
      %ifidni tmode, T  ; throughput + fld + fstp
         fprem
         fstp st0
         fld st1
      %elifidni tmode, L  ; latency + fadd
         fprem1
         fadd st0,st2
      %endif
   %endmacro

%elifidni instruct, fptan

   %macro testinit2 0
      %ifidni tcase, best
         fldz
      %else
         fld qword [div1]    ; dividend
      %endif
   %endmacro
   %macro testafter2 0
      fstp st0
   %endmacro
   %macro testcode 0
      %ifidni tmode, T  ; throughput + fld + 2*fstp
         fld st0
         fptan
         fstp st0
         fstp st0
      %elifidni tmode, L 
         %ifidni tcase, best
            fptan                 ; latency + fstp
            fstp st1
         %else
            fptan                 ; latency + fadd
            faddp st1,st0
         %endif
      %endif
   %endmacro

%elifidni instruct, frndint

   %macro testinit2 0
      fldpi
   %endmacro
   %macro testafter2 0
      fstp st0
   %endmacro
   %macro testcode 0
      %ifidni tmode, T  ; throughput + fld + fstp
         fld st0
         frndint
         fstp st0
      %elifidni tmode, L 
         frndint
      %endif
   %endmacro

%elifidni instruct, fscale

   %macro testinit2 0
      fld1
      fld qword [div1]
   %endmacro
   %macro testafter2 0
      fstp st0
      fstp st0
   %endmacro
   %macro testcode 0
      %ifidni tmode, T  ; throughput + fld + fstp
         fld st0
         fscale
         fstp st0
      %elifidni tmode, L 
         fscale
      %endif
   %endmacro


%elifidni instruct, fsincos

   %macro testinit2 0
      %ifidni tcase, best
         fldz
      %else
         fld qword [div1]    ; dividend
      %endif
   %endmacro
   %macro testafter2 0
      fstp st0
   %endmacro
   %macro testcode 0
      %ifidni tmode, T  ; throughput + fld + 2*fstp
         fld st0
         fsincos
         fstp st0
         fstp st0
      %elifidni tmode, L 
         fsincos
         faddp st1,st0
      %endif
   %endmacro

%elifidni instruct, fsin

   %macro testinit2 0
      %ifidni tcase, best
         fldz
         fldz
      %else
         fld qword [div1]    ; dividend
         fld st0
      %endif
   %endmacro
   %macro testafter2 0
      fstp st0
      fstp st0
   %endmacro
   %macro testcode 0
      %ifidni tmode, T  ; throughput + fld + fstp
         fld st0
         fsin
         fstp st0
      %elifidni tmode, L  ; latency + fadd
         fsin
         fadd st0,st1
      %endif
   %endmacro

%elifidni instruct, fsqrt

   %macro testinit2 0
      %ifidni tcase, best
         fldz
         fldz
      %else
         fld qword [div2]    ; divisor
         fld st0
      %endif
   %endmacro
   %macro testafter2 0
      fstp st0
      fstp st0
   %endmacro
   %macro testcode 0
      %ifidni tmode, T  ; throughput + fld + fstp
         fld st0
         fsqrt
         fstp st0
      %elifidni tmode, L  ; latency + fadd
         fsqrt
         fadd st0,st1
      %endif
   %endmacro

%elifidni instruct, ftst

   %macro testinit2 0
      fldpi
      fldpi
   %endmacro
   %macro testafter2 0
      fstp st0
      fstp st0
   %endmacro
   %macro testcode 0
      %ifidni tmode, T  ; throughput
         ftst
      %elifidni tmode, L  ; latency + fnstsw + test + fcmov
         ftst
         fnstsw ax
         test ah, 1
         fcmove st0,st1
      %endif
   %endmacro

%elifidni instruct, fxam

   %macro testinit2 0
      fldpi
      fldpi
   %endmacro
   %macro testafter2 0
      fstp st0
      fstp st0
   %endmacro
   %macro testcode 0
      %ifidni tmode, T  ; throughput
         fxam
      %elifidni tmode, L  ; latency + fnstsw + test + fcmov
         fxam
         fnstsw ax
         test ah, 1
         fcmove st0,st1
      %endif
   %endmacro

%elifidni instruct, fxch

   %define repeat1 1000
   %define repeat2 1
   %macro testinit2 0
      fldpi
      fldpi
      fldpi
   %endmacro
   %macro testafter2 0
      fstp st0
      fstp st0
      fstp st0
   %endmacro
   %macro testcode 0
      %rep 50
         fxch st1
         fxch st2
      %endrep
   %endmacro

%elifidni instruct, fxtract

   %macro testinit2 0
      fldpi
   %endmacro
   %macro testafter2 0
      fstp st0
   %endmacro
   %macro testcode 0
      %ifidni tmode, T  ; throughput + fld + 2*fstp
         fld st0
         fxtract
         fstp st0
         fstp st0
      %elifidni tmode, L  ; latency + fadd
         fxtract
         faddp st1,st0
      %endif
   %endmacro

%elifidni instruct, fyl2x

   %macro testinit2 0
      %ifidni tcase, best
         fld1
         fld1
      %else
         fld qword [div1] 
         fld st0
      %endif
   %endmacro
   %macro testafter2 0
      fstp st0
      fstp st0
   %endmacro
   %macro testcode 0
      %ifidni tmode, T  ; throughput + 2*fld + fstp
         fld st0
         fld st1
         instruct
         fstp st0
      %elifidni tmode, L  ; latency + fld
         fld st1
         instruct
      %endif
   %endmacro

%elifidni instruct, fyl2xp1

   %macro testinit3 0
      %ifidni tcase, best
         fld1
         fld1
      %else
         fld qword [div2] 
         fld st0
      %endif
   %endmacro
   %macro testafter1 0
      fstp st0
      fstp st0
   %endmacro
   %macro testcode 0
      %ifidni tmode, T  ; throughput + 2*fld + fstp
         fld st0
         fld st1
         instruct
         fstp st0
      %elifidni tmode, L  ; latency + fld
         fld st0
         instruct
      %endif
   %endmacro



%else  ; fnop, fincstp, fdecstp, fwait, fninit

   %macro testinit2 0
      fld1
   %endmacro
   %macro testafter2 0
      fstp st0
   %endmacro

%endif

