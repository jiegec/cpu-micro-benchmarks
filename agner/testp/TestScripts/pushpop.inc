;----------------------------------------------------------------------------
;                       pushpop.inc                         2013-08-26 Agner Fog
;
;                PMC Test program for push and pop instructions
;                           YASM syntax
;
; The following macros can be defined on the command line or in include files:
; 
; instruct:     The name of the instruction to test: push or pop
; 
; oper:         I:  immediate operand
;               R:  register operand
;               M:  memory operand
;               SP: operand = stack pointer
;               F:  operand = flags
;
; regsize:      Register size: 32, 64. Default = 64
;
; nthreads:     Number of simultaneous threads (default = 1)
; 
; (c) Copyright 2013 by Agner Fog. GNU General Public License www.gnu.org/licenses
;-----------------------------------------------------------------------------
; Define any undefined macros

%ifndef instruct
   %define instruct  push  ; default instruction is NOP
%endif

%ifndef tmode
   %define tmode  L          ; default: measure latency
%endif

%ifndef oper                 ; default: operand = register
   %define oper   R
%endif


;##############################################################################
;#
;#                 Test code macro
;#
;##############################################################################


%macro blockports 0
; unused
%endmacro


%IFIDNI   oper, I
   %define op  999
%ELIFIDNI oper, R
   %if regsize == 32
      %define op  ebx
   %else
      %define op  rbx
   %endif
%ELIFIDNI oper, M
   %if regsize == 32
      %define op  dword [esi]
   %else
      %define op  qword [rsi]
   %endif
%ELIFIDNI oper, SP
   %if regsize == 32
      %define op  esp
   %else
      %define op  rsp
   %endif
%ELIFIDNI oper, F
   %define op  
%ENDIF


%define repcount 50     ; repeat count

; main testcode macro
%macro testcode 0

   %if regsize == 32
      movd xmm0, esp    ; save stack pointer
        
      %IFIDNI instruct, pop
         %IFIDNI oper, F
            %rep repcount
               pushfd
            %endrep
         %ELSE
            sub esp, repcount*4
            mov [esp],esp   ; value for pop esp
         %ENDIF
      %ENDIF

      movd xmm1, esp    ; save stack pointer again
        
      mov ebp, 100000/repcount
      align 64
      Testloop1:

      %IFIDNI instruct, push

         %rep repcount
            %IFIDNI oper, F
               pushfd
            %ELSE
               push op
            %ENDIF
         %endrep

      %ELIFIDNI instruct, pop

         %rep repcount
            %IFIDNI oper, F
               popfd
            %ELSE
               pop op
            %ENDIF
         %endrep

      %ELSE
         %error unknown instruction
      %ENDIF

      movd esp, xmm1    ; restore stack pointer
      dec ebp
      jnz Testloop1

      movd esp, xmm0    ; restore stack pointer again

   %else                ; regsize == 64

      mov  r10,  rsp    ; save stack pointer
        
      %IFIDNI instruct, pop
         %IFIDNI oper, F
            %rep repcount
               pushfq
            %endrep
         %ELSE
            sub rsp, repcount*8
            mov [rsp],rsp   ; value for pop rsp
         %ENDIF
      %ENDIF

      mov  r11,  rsp    ; save stack pointer again
        
      mov ebp,100000/repcount
      align 64
      Testloop1:

      %IFIDNI instruct, push

         %rep repcount
            %IFIDNI oper, F
               pushfq
            %ELSE
               push op
            %ENDIF
         %endrep

      %ELIFIDNI instruct, pop

         %rep repcount
            %IFIDNI oper, F
               popfq
            %ELSE
               pop op
            %ENDIF
         %endrep

      %ELSE
         %error unknown instruction
      %ENDIF

      mov rsp, r11
      dec ebp
      jnz Testloop1

      mov rsp, r10
   
   %endif

%endmacro ; testcode

; disable default test loops
%define repeat1 1
%define repeat2 1

