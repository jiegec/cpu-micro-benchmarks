; memcpy.inc                                                       2016-11-02

; Run PMCTest for for different implementations of memcpy

; (c) 2012-2016 by Agner Fog. GNU General Public License www.gnu.org/licenses

; Parameters:
;
; mversion:   version of memcpy function
;
; dsize:      Size of memory block to move
; srcoff:     Source offset relative to cache line (64B)
; dstoff:     Destination offset relative to cache line (64B)

; define external memcpy function
extern mversion

; data size for each thread
%define threaddatasize ((dsize*2+10FFh) & (-100h))

; allocate data for all threads
%macro testdata 0
   times (threaddatasize * nthreads)  DB 0
%endmacro

; main testcode macro
%macro testcode 0

   imul rax, r15, threaddatasize
   lea rsi, [UserData + rax]
   lea rdi, [rsi + ((dsize+7FH)&(-80H))+100H+dstoff]
   mov edx, dsize
   call mversion

%endmacro

; default test loops
%define repeat1 100
%define repeat2 1
