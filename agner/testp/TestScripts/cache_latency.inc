; cache_stride.inc                                       2013-04-27 Agner Fog

; Test cache access time
; (c) 2013 by Agner Fog. GNU General Public License www.gnu.org/licenses
;
; Parameters:
;
; memsize:     Total size of memory block travelled, bytes
;
; stride:      Stride between accesses, bytes
;
; tmode:       Test mode:
;              0:    linear walk
;              1:    random walk

; extern "C" char * shuffle(int listlen, int stride, int seed) {
; extern "C" char * AllocateBuffers(size_t bufferlen, int listlen);
; extern "C" void DeAllocateBuffers();
extern shuffle 
extern AllocateBuffers
extern DeAllocateBuffers

%ifdef nthreads
  %if nthreads > 1
    %error only one thread possible
  %endif
%else
  %define nthreads 1
%endif

%define accesses (memsize/stride)


; allocate memory and make shuffled list
%macro testinit1 0

%if tmode == 0    ; linear access
        xor     eax, eax
%else              ; random access. get seed
        rdtsc
        test    eax, eax
        cmovz   eax, esp           ; make sure seed != 0
%endif        
%if     WINDOWS
        mov     rcx, accesses
        mov     edx, stride
        mov     r8d, eax
%else   ; Unix
        mov     rdi, accesses
        mov     esi, stride
        mov     edx, eax
%endif
        call    shuffle            ; allocate memory, make shuffled linked list
        mov     rsi, rax           ; point to first list node
%endmacro


%macro testafter3 0
        ; free allocated memory
        call    DeAllocateBuffers
%endmacro

; main testcode macro
%macro testcode 0

%if accesses > 0x400
        mov ebp, accesses / 0x400
LL:
%rep 0x400
        mov     psi, [psi]   ; walk through linked list
%endrep
        dec     ebp
        jnz     LL
        
%else  ; accesses <= 0x400

%rep accesses
        mov     psi, [psi]   ; walk through linked list
%endrep

%endif

%endmacro


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
                DD   UopCol      ; denominator (0 = clock, 1 = first PMC, etc., -1 = none)
                DD   0x3F800000  ; 1.0
                
TempOut         DD   0           ; 6 = float
                DD   0
RatioOutTitle DQ   Heading1      ; column heading
TempOutTitle  DQ   Heading2      ; column heading                
                
%endmacro       



; disable  test loops
%define repeat1 1
%define repeat2 1
