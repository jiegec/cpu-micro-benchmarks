; cache_stride.inc                                       2013-04-27 Agner Fog

; Test cache access time
; (c) 2013 by Agner Fog. GNU General Public License www.gnu.org/licenses
;
; Parameters:
;
; allocsize:   Amount of memory allocated, kb
;
; blocksize:   Size of memory block to access, kb
;
; stride:      Stride between accesses, bytes
;
; tmode:       Test mode:
;              R:    read only
;              W:    write only
;              RW:   read, then write to same address
;              R_W:  read and write to different address
;              RR_W: two reads and one write to different address
;              RW:   read, then write to same address
;              NTR:  non-temporal read
;              NTW:  non-temporal write

%ifndef alignby
%define alignby 64
%endif

; extern "C" char * AllocateBufferT(size_t bufferlen);
; extern "C" void DeAllocateBufferT(char *);
extern AllocateBufferT
extern DeAllocateBufferT


; allocation of memory
%macro testinit1 0

    mov rcx, (allocsize*1024) + alignby
    mov rdi, rcx
    call    AllocateBufferT
    mov  r11, rax                  ; remember address
    ; align
    mov ebx, alignby
    lea rax, [rax+rbx-1]
    neg rbx
    and rax, rbx
    mov rsi, rax
    ; zero memory block
    mov rdi, rax
    mov rcx, allocsize * 1024
    xor eax, eax
    cld
    rep stosb
    mov rdi, rsi
%endmacro

; main testcode macro
%macro testcode 0
    xor eax, eax
    mov rcx, blocksize * 1024
    lea rsi, [rdi+rcx]  ; end of block
    neg rcx
    
%ifidni tmode, R  ; read only
    
LL: ; access loop
    mov eax, [rsi+rcx]
    mov eax, [rsi+rcx+stride]
    mov eax, [rsi+rcx+stride*2]
    mov eax, [rsi+rcx+stride*3]
    add rcx, stride*4
    js LL
    
%elifidni tmode, W  ; write only

LL: ; access loop
    mov [rsi+rcx], eax
    mov [rsi+rcx+stride], eax
    mov [rsi+rcx+stride*2], eax
    mov [rsi+rcx+stride*3], eax
    add rcx, stride*4
    js LL

%elifidni tmode, RW  ; read, then write to same address

LL: ; access loop
    inc dword [rsi+rcx]
    inc dword [rsi+rcx+stride]
    inc dword [rsi+rcx+stride*2]
    inc dword [rsi+rcx+stride*3]
    add rcx, stride*4
    js LL
    
%elifidni tmode, R_W   ; read and write to different address

LL: ; access loop
    mov eax, [rsi+rcx]
    mov      [rsi+rcx+20h], ebx
    mov eax, [rsi+rcx+stride]
    mov      [rsi+rcx+stride+20h], ebx
    mov eax, [rsi+rcx+stride*2]
    mov      [rsi+rcx+stride*2+20h], ebx
    mov eax, [rsi+rcx+stride*3]
    mov      [rsi+rcx+stride*3+20h], ebx
    add rcx, stride*4
    js LL
    
%elifidni tmode, RR_W     ; two reads and one write to different address
LL: ; access loop
    mov eax, [rsi+rcx]
    mov edx, [rsi+rcx+10h]
    mov      [rsi+rcx+20h], ebx
    mov eax, [rsi+rcx+stride]
    mov edx, [rsi+rcx+stride+10h]
    mov      [rsi+rcx+stride+20h], ebx
    mov eax, [rsi+rcx+stride*2]
    mov edx, [rsi+rcx+stride*2+10h]
    mov      [rsi+rcx+stride*2+20h], ebx
    mov eax, [rsi+rcx+stride*3]
    mov edx, [rsi+rcx+stride*3+10h]
    mov      [rsi+rcx+stride*3+20h], ebx
    add rcx, stride*4
    js LL

%elifidni tmode, NTR  ; non-temporal read

LL: ; access loop
    movntdqa xmm0, [rsi+rcx]
    movntdqa xmm0, [rsi+rcx+stride]
    movntdqa xmm0, [rsi+rcx+stride*2]
    movntdqa xmm0, [rsi+rcx+stride*3]
    add rcx, stride*4
    js LL

%elifidni tmode, NTW  ; non-temporal write

LL: ; access loop
    movnti [rsi+rcx], eax
    movnti [rsi+rcx+stride], eax
    movnti [rsi+rcx+stride*2], eax
    movnti [rsi+rcx+stride*3], eax
    add rcx, stride*4
    js LL
    
%else 
  
    %error unknown tmode
    
%endif
    
%endmacro

; disable  test loops
%define repeat1 1
%define repeat2 1


;##############################################################################
;#
;#                 Extra calculations for convenience
;#
;##############################################################################

%macro extraoutput 0
Heading1        DB   "clock/access", 0
Heading2        DB   "?", 0
align 8

; Decide which column to base clock count and uop count on
%ifidni CPUbrand,Intel
%define ClockCol  1   ; use core clock cycles on Intel processors
%define UopCol    4   ; 3 or 4, which has the best uop count?

%else                 ; All other CPU brands:
%define ClockCol  0   ; use RDTSC clock on all other processors
%define UopCol    2   ; which column has uop count
%endif

RatioOut        DD   2           ; 0: no ratio output, 1 = int, 2 = float
                DD   ClockCol    ; numerator (0 = clock, 1 = first PMC, etc., -1 = none)
                DD   -1          ; denominator (0 = clock, 1 = first PMC, etc., -1 = none)
                DD   0           ; factor calculated later, int or float according to RatioOut[0]
                
TempOut         DD   0           ; 6 = float
                DD   0
RatioOutTitle DQ   Heading1      ; column heading
TempOutTitle  DQ   Heading2      ; column heading                
                
%endmacro       


%macro testafter3 0

    ; remember to free buffers
    mov     rdi, r11
    mov     rcx, r11
    call    DeAllocateBufferT
    
    ; calculate number of accesses
    mov     eax, blocksize * (1024 / stride)
    mov     ebx, 3F800000H   ; 1.0
    movd    xmm0, ebx
    cvtsi2ss xmm1,eax
    divss   xmm0,xmm1        ; 1.0 / accesses
    movss   [RatioOut + 12], xmm0     ; Store as RatioOut[3]
    
%endmacro


