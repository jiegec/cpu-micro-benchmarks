; daxpy.inc                                                           2016-11-02 Agner Fog

; Run PMCTest for various implementations of DAXPY algorithm
; (c) 2012-2016 by Agner Fog. GNU General Public License www.gnu.org/licenses


; Parameters:
;
; tcase:  test case:
;         1:  SSE2, 128 bit
;         2:  AVX, 256 bit
;         3:  FMA3, 128 bit
;         4:  FMA3, 256 bit
;         5:  FMA4, 128 bit
;         6:  FMA4, 256 bit
;         7:  AVX512, 512 bit
;
; ndat:   number of data elements


%ifndef tcase
   %define tcase  1
%endif

%ifndef ndat
   %define ndat  4000
%endif

%ifndef nthreads
   %define nthreads  1    ; default number of threads = 1
%endif


; define test data
%macro testdata 0
   %assign datasize1  (ndat*8*2+500h)
   times (datasize1*nthreads) db 0
%endmacro

; calculate data addresses
%macro testinit1 0
   imul eax, r15d, datasize1 ; separate data for each thread
   lea r8, [UserData]
   add r8, rax
   lea r9, [r8+ndat*8+300h]
%endmacro

; reset pointers and counters before each test run
%macro testinit3 0
   mov ecx, ndat*8
   mov eax, ecx
   neg rax
   lea rsi, [r8+rcx]
   lea rdi, [r9+rcx]
   xorps xmm2, xmm2            ; initialize DA
%endmacro


; main testcode macro
%macro testcode 0

   %if tcase == 1     ; SSE2

      L1:
      movapd xmm1, [rsi+rax]
      mulpd  xmm1, xmm2
      addpd  xmm1, [rdi+rax]
      movapd [rdi+rax], xmm1
      add rax, 16                                ; Add size of two elements to index
      jl L1                                      ; Loop

   %elif tcase == 2     ; AVX

      L1:
      vmulpd  ymm1, ymm2, yword [rsi+rax]        ; X[i] * (-DA), X[i+1] * (-DA)
      vaddpd  ymm1, ymm1, yword [rdi+rax]        ; Y[i]-X[i]*DA, Y[i+1]-X[i+1]*DA
      vmovapd yword[rdi+rax],ymm1
      add rax, 32                                ; Add size of four elements to index
      jl L1                                      ; Loop

   %elif tcase == 3     ; FMA3, 128 bit

      L1:
      vmovapd xmm1, [rsi+rax]
      vfnmadd231pd xmm1, xmm2, [rdi+rax]
      vmovapd [rdi+rax],xmm1
      add rax, 16                                ; Add size of two elements to index
      jl L1                                      ; Loop

   %elif tcase == 4     ; FMA3, 256 bit

      L1:
      vmovapd ymm1, [rsi+rax]
      vfnmadd231pd ymm1, ymm2, [rdi+rax]
      vmovapd [rdi+rax],ymm1
      add rax, 32                                ; Add size of four elements to index
      jl L1                                      ; Loop

   %elif tcase == 5     ; FMA4, 128 bit

      L1:
      vmovapd xmm1, [rsi+rax]
      vfnmaddpd xmm1, xmm1, xmm2, [rdi+rax]
      vmovapd [rdi+rax],xmm1
      add rax, 16                                ; Add size of two elements to index
      jl L1                                      ; Loop

   %elif tcase == 6     ; FMA4, 256 bit

      L1:
      vmovapd ymm1, [rsi+rax]
      vfnmaddpd ymm1, ymm1, ymm2, [rdi+rax]
      vmovapd [rdi+rax],ymm1
      add rax, 32                                ; Add size of four elements to index
      jl L1                                      ; Loop

   %elif tcase == 7     ; AVX512, 512 bit

      L1:
      vmovapd zmm1, [rsi+rax]
      vfnmadd231pd zmm1, zmm2, [rdi+rax]
      vmovapd [rdi+rax],zmm1
      add rax, 64                                ; Add size of 8 elements to index
      jl L1                                      ; Loop

   %endif

%endmacro


; default test loops
%define repeat1 100
%define repeat2 1

