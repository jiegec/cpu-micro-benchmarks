; taylor.inc                                         2013-07-10 Agner Fog

; Run PMCTest for various implementations of the Taylor expansion algorithm
; (c) 2012 - 2013 by Agner Fog. GNU General Public License www.gnu.org/licenses

; Parameters:
;
; tcase:  test case:
;         1:  SSE2
;         2:  AVX
;         3:  FMA3, 128 bit, 4 terms per iteration
;         4:  FMA3, 256 bit, 8 terms per iteration
;         5:  FMA4, 128 bit, 4 terms per iteration
;         6:  FMA4, 256 bit, 8 terms per iteration
;
; ndat:   number of terms, defalt = 16


%ifndef tcase
   %define tcase  1
%endif

%ifndef ndat
   %define ndat  16
%endif

%ifndef nthreads
   %define nthreads  1    ; default number of threads = 1
%endif


; define test data
%macro testdata 0
   one dq 1.0, 1.0
   x dq 1.0, 1.0
   yresult: dq 0.0  ; result
   align 32
   coeff: 
   dq 1.0, 1.0, 0.5, 0.1666666666666666   ; 0 1 2 3
   dq 0.041666666666666, 8.3333333333E-3, 1.3888888888E-3, 1.9841269841E-4  ; 4 5 6 7
   dq 2.48015873E-5, 2.75573E-6, 2.75573E-7, 2.50521E-8 ; 8 9 10 11
   dq 2.08797E-9, 1.6059E-10, 1.14707E-11, 7.64716E-13  ; 12 13 14 15
   times 1000H  DQ 0
%endmacro

; main testcode macro
%macro testcode 0

   %if tcase == 1     ; SSE2
      ; Example 12.9d. Taylor expansion, quadruple steps

      ; in which order should I write the operands in the comments?
      movsd   xmm4, [x]                  ; x
      mulsd   xmm4, xmm4                 ; x^2
      movlhps xmm4, xmm4                 ; x^2, x^2
      ;movapd  xmm2, oword [one]         ; xmm2(L)=1.0, xmm2(H)=x
      movsd   xmm2, [one]
      movhpd  xmm2, [x]                  ; xmm2(L)=1.0, xmm2(H)=x
      movapd  xmm1, xmm2                 ; xmm1 = x, 1
      mulpd   xmm2, xmm4                 ; xmm2 = x^3, x^2
      mulpd   xmm4, xmm4                 ; xmm4 = x^4, x^4
      xorps   xmm5, xmm5                 ; xmm5 = sum. init. to 0
      xorps   xmm6, xmm6                 ; xmm6 = sum. init. to 0
      lea     rax,  [coeff]              ; point to c[i]
      lea     rcx,  [rax+ndat*8]         ; end of c[i]
      L1: 
      movapd  xmm3, xmm1                 ; copy x^(i+1), x^i
      movapd  xmm0, xmm2                 ; copy x^(i+3), x^(i+2)
      mulpd   xmm1, xmm4                 ; x^(i+5), x^(i+4)
      mulpd   xmm2, xmm4                 ; x^(i+7), x^(i+6)    
      mulpd   xmm3, [rax]                ; term(i+1), term(i)
      mulpd   xmm0, [rax+16]             ; term(i+3), term(i+2)    
      addpd   xmm5, xmm3                 ; add to sum
      addpd   xmm6, xmm0                 ; add to sum    
      add     rax,  32                   ; point to c[i+2]
      cmp     rax,  rcx                  ; stop at end of list
      jb      L1                         ; loop
      addpd   xmm5, xmm6                 ; join two accumulators
      haddpd  xmm5, xmm5                 ; final sum
      movsd [yresult], xmm5
    
   %elif tcase == 2                      ; AVX version. Calculates 8 terms

      ; register use:
      ; ymm0 sum0
      ; ymm1 sum1
      ; ymm2 1, x, x^2, x^3
      ; ymm3 x^4, x^5, x^6, x^7
      ; ymm4 x^8, x^8, x^8, x^8
      ; ymm5 scratch
      ; rax pointer to end of coeff
      ; rcx negative loop counter

      vmovddup xmm2, [x]               ; x, x
      vmulpd xmm5, xmm2, xmm2          ; x^2, x^2
      vmulpd xmm4, xmm5, xmm5          ; x^4, x^4
      vmovsd xmm2, [one]               ; 1
      vmovhpd xmm2, xmm2, [x]          ; 1, x
      vmulpd xmm0, xmm2, xmm5          ; x^2, x^3
      vinsertf128 ymm2, ymm2, xmm0, 1  ; 1, x, x^2, x^3
      vinsertf128 ymm4, ymm4, xmm4, 1  ; x^4, x^4, x^4, x^4
      vmulpd ymm3, ymm2, ymm4          ; x^4, x^5, x^6, x^7
      vmulpd ymm4, ymm4, ymm4          ; x^8, x^8, x^8, x^8
      vxorps xmm0, xmm0                ; sum0 = 0
      vxorps xmm1, xmm1                ; sum1 = 0

      lea     rax,  [coeff+ndat*8]     ; point to end of coeff[i]
      mov     rcx, -ndat*8             ; counter
    
      jmp L2
    
      align 32    

      L1: 
      vmulpd ymm2, ymm2, ymm4   ; multiply powers of x by x^8
      vmulpd ymm3, ymm3, ymm4   ; multiply powers of x by x^8
      L2: 
      vmulpd ymm5, ymm2, [rax+rcx]  ; first four terms
      vaddpd ymm0, ymm0, ymm5   ; sum0
      vmulpd ymm5, ymm3, [rax+rcx+32]  ; next four terms
      vaddpd ymm1, ymm1, ymm5   ; sum1
      add rcx, 64
      jl  L1

      ; make total sum
      vaddpd ymm0, ymm0, ymm1   ; join two sums
      vextractf128 xmm5, ymm0, 1 ; get high part
      vaddpd xmm0, xmm0, xmm5
      vhaddpd xmm0, xmm0, xmm0  ; final sum
      vmovsd [yresult], xmm0        ; store result
      vzeroupper
      
   %elif tcase == 3                      ; FMA3 version, 128 bit. Calculates 4 terms (limited by latency)

      ; register use:
      ; xmm0 sum0
      ; xmm1 sum1
      ; xmm2 1, x  
      ; xmm3 x^2, x^3 
      ; xmm4 x^4, x^4 
      ; xmm5 scratch
      ; rax pointer to end of coeff
      ; rcx negative loop counter

      vmovddup xmm2, [x]               ; x, x
      vmulpd xmm5, xmm2, xmm2          ; x^2, x^2
      vmulpd xmm4, xmm5, xmm5          ; x^4, x^4
      vmovsd xmm1, [one]               ; 1
      vmovsd xmm2, xmm2, xmm1          ; 1, x
      vmulpd xmm3, xmm2, xmm5          ; x^2, x^3
      vxorps xmm0, xmm0                ; sum0 = 0
      vxorps xmm1, xmm1                ; sum1 = 0

      lea     rax,  [coeff+ndat*8]     ; point to end of coeff[i]
      mov     rcx, -ndat*8             ; counter    
      jmp L2
    
      align 32 
      L1: 
      vmulpd xmm2, xmm2, xmm4          ; multiply powers of x by x^8
      vmulpd xmm3, xmm3, xmm4          ; multiply powers of x by x^8
      L2:
      vfmadd231pd xmm0, xmm2, [rax+rcx]     ; first two terms
      vfmadd231pd xmm1, xmm3, [rax+rcx+16]  ; next two terms
      add rcx, 32
      jl  L1

      ; make total sum
      vaddpd xmm0, xmm0, xmm1          ; join two sums
      vhaddpd xmm0, xmm0, xmm0         ; final sum
      vmovsd [yresult], xmm0           ; store result

   %elif tcase == 4                    ; FMA3 version, 256 bit. Calculates 8 terms

      ; register use:
      ; ymm0 sum0
      ; ymm1 sum1
      ; ymm2 1, x, x^2, x^3
      ; ymm3 x^4, x^5, x^6, x^7
      ; ymm4 x^8, x^8, x^8, x^8
      ; ymm5 scratch
      ; rax pointer to end of coeff
      ; rcx negative loop counter

      vmovddup xmm2, [x]               ; x, x
      vmulpd xmm5, xmm2, xmm2          ; x^2, x^2
      vmulpd xmm4, xmm5, xmm5          ; x^4, x^4
      vmovsd xmm2, [one]               ; 1
      vmovhpd xmm2, xmm2, [x]          ; 1, x
      vmulpd xmm0, xmm2, xmm5          ; x^2, x^3
      vinsertf128 ymm2, ymm2, xmm0, 1  ; 1, x, x^2, x^3
      vinsertf128 ymm4, ymm4, xmm4, 1  ; x^4, x^4, x^4, x^4
      vmulpd ymm3, ymm2, ymm4          ; x^4, x^5, x^6, x^7
      vmulpd ymm4, ymm4, ymm4          ; x^8, x^8, x^8, x^8
      vxorps xmm0, xmm0                ; sum0 = 0
      vxorps xmm1, xmm1                ; sum1 = 0

      lea     rax,  [coeff+ndat*8]     ; point to end of coeff[i]
      mov     rcx, -ndat*8             ; counter    
      jmp L2
    
      align 32    
      L1: 
      vmulpd ymm2, ymm2, ymm4          ; multiply powers of x by x^8
      vmulpd ymm3, ymm3, ymm4          ; multiply powers of x by x^8
      L2:
      vfmadd231pd ymm0, ymm2, [rax+rcx]    ; first four terms
      vfmadd231pd ymm1, ymm3, [rax+rcx+32] ; next four terms
      add rcx, 64
      jl  L1

      ; make total sum
      vaddpd ymm0, ymm0, ymm1   ; join two sums
      vextractf128 xmm5, ymm0, 1 ; get high part
      vaddpd xmm0, xmm0, xmm5
      vhaddpd xmm0, xmm0, xmm0  ; final sum
      vmovsd [yresult], xmm0        ; store result
      vzeroupper
            
   %elif tcase == 5                      ; FMA4 version, 128 bit. Calculates 4 terms (limited by latency)

      ; register use:
      ; xmm0 sum0
      ; xmm1 sum1
      ; xmm2 1, x  
      ; xmm3 x^2, x^3 
      ; xmm4 x^4, x^4 
      ; xmm5 scratch
      ; rax pointer to end of coeff
      ; rcx negative loop counter

      vmovddup xmm2, [x]               ; x, x
      vmulpd xmm5, xmm2, xmm2          ; x^2, x^2
      vmulpd xmm4, xmm5, xmm5          ; x^4, x^4
      vmovsd xmm1, [one]               ; 1
      vmovsd xmm2, xmm2, xmm1          ; 1, x
      vmulpd xmm3, xmm2, xmm5          ; x^2, x^3
      vxorps xmm0, xmm0                ; sum0 = 0
      vxorps xmm1, xmm1                ; sum1 = 0

      lea     rax,  [coeff+ndat*8]     ; point to end of coeff[i]
      mov     rcx, -ndat*8             ; counter    
      jmp L2
    
      align 32 
      L1: 
      vmulpd xmm2, xmm2, xmm4          ; multiply powers of x by x^8
      vmulpd xmm3, xmm3, xmm4          ; multiply powers of x by x^8
      L2:
      vfmaddpd xmm0, xmm2, [rax+rcx], xmm0     ; first two terms
      vfmaddpd xmm1, xmm3, [rax+rcx+16], xmm1  ; next two terms
      add rcx, 32
      jl  L1

      ; make total sum
      vaddpd xmm0, xmm0, xmm1          ; join two sums
      vhaddpd xmm0, xmm0, xmm0         ; final sum
      vmovsd [yresult], xmm0           ; store result

   %elif tcase == 6                    ; FMA4 version, 256 bit. Calculates 8 terms

      ; register use:
      ; ymm0 sum0
      ; ymm1 sum1
      ; ymm2 1, x, x^2, x^3
      ; ymm3 x^4, x^5, x^6, x^7
      ; ymm4 x^8, x^8, x^8, x^8
      ; ymm5 scratch
      ; rax pointer to end of coeff
      ; rcx negative loop counter

      vmovddup xmm2, [x]               ; x, x
      vmulpd xmm5, xmm2, xmm2          ; x^2, x^2
      vmulpd xmm4, xmm5, xmm5          ; x^4, x^4
      vmovsd xmm2, [one]               ; 1
      vmovhpd xmm2, xmm2, [x]          ; 1, x
      vmulpd xmm0, xmm2, xmm5          ; x^2, x^3
      vinsertf128 ymm2, ymm2, xmm0, 1  ; 1, x, x^2, x^3
      vinsertf128 ymm4, ymm4, xmm4, 1  ; x^4, x^4, x^4, x^4
      vmulpd ymm3, ymm2, ymm4          ; x^4, x^5, x^6, x^7
      vmulpd ymm4, ymm4, ymm4          ; x^8, x^8, x^8, x^8
      vxorps xmm0, xmm0                ; sum0 = 0
      vxorps xmm1, xmm1                ; sum1 = 0

      lea     rax,  [coeff+ndat*8]     ; point to end of coeff[i]
      mov     rcx, -ndat*8             ; counter    
      jmp L2
    
      align 32    
      L1: 
      vmulpd ymm2, ymm2, ymm4          ; multiply powers of x by x^8
      vmulpd ymm3, ymm3, ymm4          ; multiply powers of x by x^8
      L2:
      vfmaddpd ymm0, ymm2, [rax+rcx], ymm0     ; first four terms
      vfmaddpd ymm1, ymm3, [rax+rcx+32], ymm1  ; next four terms
      add rcx, 64
      jl  L1

      ; make total sum
      vaddpd ymm0, ymm0, ymm1   ; join two sums
      vextractf128 xmm5, ymm0, 1 ; get high part
      vaddpd xmm0, xmm0, xmm5
      vhaddpd xmm0, xmm0, xmm0  ; final sum
      vmovsd [yresult], xmm0        ; store result
      vzeroupper

   %endif 

%endmacro


; default test loops
%define repeat2 1

