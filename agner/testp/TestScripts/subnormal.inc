; subnormal.inc                                                                 2015-11-29 AgF
;
; Run PMCTest for floating point instructions with various combinations of subnormal operands
; (c) 2012-2015 by Agner Fog. GNU General Public License www.gnu.org/licenses

; Parameters:
;
; tcase: test case (0 - 20)
;
; flushtoz:   1 for enabling flush-to-zero mode
;
; subnormalsz: 1 for enabling subnormals-are-zero mode

; enable test loops
%define repeat1 100
%define repeat2 100

%macro testdata 0  ; define test cases

        DD   1.0,       1.0           ; 0:  normal + normal = normal
        DD   00803000h, 80802000h     ; 1:  normal + normal = subnormal
        DD   1.0,       00001000h     ; 2:  normal + subnormal = normal
        DD   00800000h, 80600000h     ; 3:  normal + subnormal = subnormal
        DD   00700000h, 007f0000h     ; 4:  subnormal + subnormal = normal
        DD   0000000fh, 0000000fh     ; 5:  subnormal + subnormal = subnormal
        DD   7f7fff00h, 7f7fff01h     ; 6:  normal + normal = overflow
        DD   1.0,       7f800001h     ; 7:  normal + NAN = NAN
        DD   1.0,       7f800000h     ; 8:  normal + INF = INF
        DD   7f800000h, 7f800000h     ; 9:  INF + INF = INF
        DD   7f800000h, 0ff800000h    ; 10: INF - INF = NAN

        DD   1.0,       1.0           ; 11: normal * normal = normal
        DD   1.0E-30,   1.0E-10       ; 12: normal * normal = subnormal
        DD   1.0E+30,   00300000h     ; 13: normal * subnormal = normal
        DD   2.0,       00100000h     ; 14: normal * subnormal = subnormal
        DD   00300000h, 00300000h     ; 15: subnormal * subnormal = underflow
        DD   1.0E-30,   1.0E-30       ; 16: normal * normal = underflow
        DD   1.0E+30,   1.0E+20       ; 17: normal * normal = overflow
        DD   1.0,       7f800001h     ; 18: normal * NAN = NAN
        DD   1.0,       7f800000h     ; 19: normal * INF = INF
        DD   0.0,       7f800000h     ; 20: 0 * INF = NAN

        DD   1.2345,    31.8407       ; 21: normal / normal = normal
        DD   1.0E-30,   1.0E+10       ; 22: normal / normal = subnormal
        DD   1.0E-10,   00300000h     ; 23: normal / subnormal = normal
        DD   00200000h, 1.4           ; 24: subnormal / normal = subnormal
        DD   1.0E-20,   1.0E+30       ; 25: normal / normal = underflow
        DD   1.0E+30,   1.0E-20       ; 26: normal / normal = overflow
        DD   1.23,      7f800001h     ; 27: normal / NAN = NAN
        DD   1.23,      7f800000h     ; 28: normal / INF = 0
        DD   1.23,      0             ; 29: normal / 0 = INF
        DD   0,         0             ; 30: 0 / 0 = NAN

        times 10000H DD  0

%endmacro

%ifndef tcase
   %define tcase 1
%endif

%ifndef flushtoz
   %define flushtoz 0
%endif

%ifndef denormalsz
   %define denormalsz 0
%endif

%macro testinit1 0  ;  initialize

   mov  eax, tcase
   cmp  eax, 50
   jb   TCASEOK
   xor  eax, eax
   TCASEOK:

%if modesize == 64

   lea rdi, [UserData]           ; point rdi to selected test case
   lea rdi, [rdi+8*rax]          ;   (lea rdi,[UserData+8*rax] not allowed under Windows)
   movsd xmm1, [rdi]             ; load operands
   movsd xmm2, [rdi+4]
   lea rbx, [rsi+100H]  
   stmxcsr [rbx]
   mov eax, [rbx]
   %if flushtoz > 0
      bts eax,15                  ; set flush to zero mode
   %else
      btr eax,15
   %endif
   %if denormalsz > 0
      bts eax,6                   ; set subnormals are zero mode
   %else
      btr eax,6
   %endif
   mov [rbx], eax
   ldmxcsr [rbx]

%else           ; 32 bit mode

   lea edi, [UserData+8*eax]     ; point edi to selected test case
   movsd xmm1, [edi]             ; load operands
   movsd xmm2, [edi+4]
   lea ebx, [esi+100H]  
   stmxcsr [ebx]
   mov eax, [ebx]
   %if flushtoz > 0
      bts eax,15                  ; set flush to zero mode
   %else
      btr eax,15
   %endif
   %if denormalsz > 0
      bts eax,6                   ; set subnormals are zero mode
   %else
      btr eax,6
   %endif
   mov [ebx], eax
   ldmxcsr [ebx]

%endif

%endmacro

; main testcode macro
%macro testcode 0


   %if tcase < 11      ; addition

      movss xmm0, xmm1
      addss xmm0, xmm2

   %elif tcase < 21      ; multiplication

      movss xmm0, xmm1
      mulss xmm0, xmm2

   %else                 ; division

      movss xmm0, xmm1
      divss xmm0, xmm2

   %endif

%endmacro

