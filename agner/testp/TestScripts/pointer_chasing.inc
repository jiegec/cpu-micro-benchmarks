; pointer_chasing.inc                                                        2021-01-30 Agner Fog

; Test cache access time
; (c) 2013 - 2021 by Agner Fog. GNU General Public License www.gnu.org/licenses

; Parameters:
;
; tcase: addressing mode

; define test data
%macro testdata 0
pointers dq 0,0,0,0
%endmacro

; initialization of pointers
%macro testinit1 0
    lea rsi, [pointers]
    lea rdi, [rsi+8]
    mov [rsi], rdi
    add rdi, 8
    mov [rsi+8], rdi
    add rdi, 8
    mov [rsi+16], rdi
    add rdi, 8
    mov [rsi+24], rsi
    mov rdi, rsi
    xor ebp, ebp
    xor r8d, r8d
    mov r9d, 1
%endmacro


; main testcode macro
%macro testcode 0
    %if tcase == 1
        mov rdi, [rdi]            ; base pointer only
    %elif tcase == 2
        mov rdi, [rdi + rbp * 2]  ; base + scaled index
    %elif tcase == 3
        mov rdi, [r8 + rdi * 1]   ; base + scaled index. latency through index
    %elif tcase == 4
        mov rdi, [rdi + r9 * 4 - 4]  ; base + scaled index + offset
    %endif
%endmacro

; disable default test loops
%define repeat1 1000
%define repeat2 100
