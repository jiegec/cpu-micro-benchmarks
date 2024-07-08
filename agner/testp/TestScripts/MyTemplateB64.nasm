;----------------------------------------------------------------------------
;                       TemplateB64.nasm                2022-10-28 Agner Fog
;
;                PMC Test program for multiple threads
;                           NASM syntax
;
; This file is a replacement for the file PMCTestB64.nasm where relevant 
; parts are coded as replaceable macros. This is useful for automated test
; scripts where the macro definitions are inserted on the command line or
; as included files.
;
; The following defines and macros can be defined on the command line or in include files:
; 
; instruct:      The name of a single instruction to test (define or macro). Default = nop
;
; instruct2:     Extra line of code following instruction. Default = nothing
;
; regsize:       Register size: 8, 16, 32, 64, 128, 256, 512. Default = 32
;                (Legacy code has regsize=65 indicating mmx register)
;
; regtype:       Register type: r = general purpose register, h = high 8-bit register,
;                v = vector register 128 bits and bigger, mmx = mmx register, k = mask register. 
;                Default is r for regsize <= 64, v for regsize >= 128
;
; numop:         Number of register operands (0 - 3). Default = 0
;
; numimm:        Number of immediate operands (0 - 1). Default = 0
;
; immvalue:      Value of first immediate operand. Default = 0
;
; testcode:      A multi-line macro executing any piece of test code. (Replaces instruction and numop); 
;
; testdata:      Macro defining any static data needed for test. Default = 1000H bytes
; 
; testinit1:     Macro with initializations before all tests. Default sets rsi to point to testdata
;
; testinit2:     Macro with initializations before each test. Default = nothing
;
; testinit3:     Macro with initializations before macro loop. Default = nothing
;
; testinitc:     Macro to call in each test before reading counters
;
; testafter1:    Macro with any cleanup to do after macro loop. Default = nothing
;
; testafter2:    Macro with any cleanup to do after repeat1 loop. Default = nothing
;
; testafter3:    Macro with any cleanup to do after all tests. Default = nothing
;
; repeat0:       Number of repetitions of whole test. Default = 8
;
; repeat1:       Repeat count for loop around testcode. Default = no loop
;
; repeat2:       Repeat count for repeat macro around testcode. Default = 100
;
; noloops:       Define this if repeat1 and repeat2 loops are contained in testcode rather than here
;
; nthreads:      Number of simultaneous threads (default = 1)
; 
; counters:      A comma-separated list of PMC counter numbers (referring to CounterDefinitions in PMCTestA.cpp)
;                Default = include "countertypes.inc"
; 
; WINDOWS:       1 if Windows operating system. Default = 0
;
; USEAVX:        1 if AVX registers used. Default = 1
;
; WARMUPCOUNT:   Set to 10000000 to get CPU into max frequency by executing dummy instructions. Default = 10000
;
; CACHELINESIZE: Size of data cache lines. Default = 64
;
; codealign:     Alignment of test code. Default = 16
; 
; See PMCTestB64.nasm and PMCTest.txt for general instructions.
; 
; (c) 2000-2018 GNU General Public License www.gnu.org/licenses
; 
;-----------------------------------------------------------------------------

%include "countertypes.inc"   ; include file defining various parameters

; Define any undefined macros

%ifndef repeat1
   %define repeat1 1
%endif

%ifndef repeat2
   %define repeat2 100
%endif

%ifndef instruct
   %define instruct  nop  ; default instruction is NOP
%endif

%ifndef instruct2
   %define instruct2
%endif

%ifndef codealign            ; default: align test code by 16
   %define codealign 16
%endif

%ifndef numop
   %define numop  0    ; default number of register operands
%endif

%ifndef immvalue
   %define immvalue  0  ; value of immediate operands
%endif

%ifndef numimm
   %define numimm  0  ; default number of immediate operands
%endif

%if numimm == 0
   %define immoperands0 
   %define immoperands1
%elif numimm == 1
   %define immoperands0   immvalue
   %define immoperands1 , immvalue
%elif numimm == 2
   %define immoperands0   immvalue , immvalue
   %define immoperands1 , immvalue , immvalue
%endif

%ifnmacro testcode
   %macro testcode 0   ; default: run instruction 100 times
      %if numop == 0
         instruct immoperands0
      %elif numop == 1
         instruct reg0 immoperands1
      %elif numop == 2
         instruct reg0, reg1 immoperands1
      %elif numop == 3
         instruct reg0, reg0, reg1 immoperands1
      %else
         %error "unknown numop"
      %endif
      instruct2
   %endmacro
%endif

; Operating system: 0 = Linux, 1 = Windows
%ifndef WINDOWS
%define  WINDOWS  0
%endif

; Warmup code. Set to 10000 to get CPU into max frequency. Set to 10000000 for AMD processors with no core clock counter
%ifndef WARMUPCOUNT
%define WARMUPCOUNT  100000
%endif

; Define cache line size (to avoid threads sharing cache lines):
%ifndef CACHELINESIZE
%define CACHELINESIZE  64
%endif

; Define whether AVX and YMM registers used
%ifndef  USEAVX
%define  USEAVX   1
%endif

; Number of repetitions of test.
%ifdef   repeat0
%define  REPETITIONS0  repeat0
%else
%define  REPETITIONS0  8
%endif

%ifdef noloops               ; put loops in testcode rather than here
%define REPETITIONS1 1
%define REPETITIONS2 1
%else
%define REPETITIONS1 repeat1
%define REPETITIONS2 repeat2
%endif

%ifndef nthreads
   %define nthreads  1    ; default number of threads = 1
%endif

%ifndef counters
   % define counters 1,9,100,150
%endif

; Define registers depending on regtype and regsize
%ifndef regtype
   %ifndef regsize
      %define regsize 32
   %endif
   %if regsize == 9
      %define regtype h
   %elif regsize < 65
      %define regtype r
   %elif regsize == 65
      %define regtype mmx
   %else
      %define regtype v
   %endif
%endif

%ifidni regtype, r
   %ifndef regsize
      %define regsize   32      ; default: define registers as 32 bit
   %endif
%elifidni regtype, h
   %ifndef regsize
      %define regsize   9       ; high 8-bit register
   %endif
%elifidni regtype, v
   %ifndef regsize
      %define regsize   128
   %endif
%elifidni regtype, mmx
   %ifndef regsize
      %define regsize   64
   %endif
%elifidni regtype, k
   %ifndef regsize
      %define regsize   16
   %endif
%else
   %error unknown register type regtype
%endif

%ifidni regtype, mmx        ; 64 bit mmx registers
   %define reg0  mm0
   %define reg1  mm1
   %define reg2  mm2
   %define reg3  mm3
   %define reg4  mm4
   %define reg5  mm5
   %define reg6  mm6
   %define reg7  mm7
 ; %define sizeptr mmword
   %define sizeptr qword
   %define numregs 8
%elifidni regtype, h       ; high 8-bit registers
   %define reg0  ah
   %define reg1  bh
   %define reg2  ch
   %define reg3  dh
   %define reg4  al
   %define reg5  bl
   %define reg6  cl
   %define reg7  dl
   %define sizeptr byte
   %define numregs 8
%elifidni regtype, k       ; mask registers, any size
   %define reg0  k1
   %define reg1  k2
   %define reg2  k3
   %define reg3  k4
   %define reg4  k5
   %define reg5  k6
   %define reg6  k7
   %define numregs 7
   %if regsize == 8
      %define sizeptr byte
   %elif regsize == 16
      %define sizeptr word
   %elif regsize == 32
      %define sizeptr dword
   %elif regsize == 64
      %define sizeptr qword
   %else
      %error unknown size for mask registers
   %endif
%elif regsize == 8             ; define registers of desired size
   %define reg0  al
   %define reg1  bl
   %define reg2  cl
   %define reg3  dl
   %define reg4  dil
   %define reg5  sil
   %define reg6  bpl
   %define reg7  r8b
   %define reg8  r9b
   %define reg9  r10b
   %define reg10 r11b
   %define sizeptr byte
   %define numregs 10
%elif regsize == 16
   %define reg0  ax
   %define reg1  bx
   %define reg2  cx
   %define reg3  dx
   %define reg4  di
   %define reg5  si
   %define reg6  bp
   %define reg7  r8w
   %define reg8  r9w
   %define reg9  r10w
   %define reg10 r11w
   %define sizeptr word
   %define numregs 10
%elif regsize == 32
   %define reg0  eax
   %define reg1  ebx
   %define reg2  ecx
   %define reg3  edx
   %define reg4  edi
   %define reg5  esi
   %define reg6  ebp
   %define reg7  r8d
   %define reg8  r9d
   %define reg9  r10d
   %define reg10 r11d
   %define sizeptr dword
   %define numregs 10
%elif regsize == 64
   %define reg0  rax
   %define reg1  rbx
   %define reg2  rcx
   %define reg3  rdx
   %define reg4  rdi
   %define reg5  rsi
   %define reg6  rbp
   %define reg7  r8
   %define reg8  r9
   %define reg9  r10
   %define reg10 r11
   %define sizeptr qword
   %define numregs 10
%elif regsize == 128
   %define reg0  xmm0
   %define reg1  xmm1
   %define reg2  xmm2
   %define reg3  xmm3
   %define reg4  xmm4
   %define reg5  xmm5
   %define reg6  xmm6
   %define reg7  xmm7
   %define reg8  xmm8
   %define reg9  xmm9
   %define reg10 xmm10
   %define reg11 xmm11
   %define reg12 xmm12
   %define sizeptr oword   
   %define numregs 12
%elif regsize == 256
   %define reg0  ymm0
   %define reg1  ymm1
   %define reg2  ymm2
   %define reg3  ymm3
   %define reg4  ymm4
   %define reg5  ymm5
   %define reg6  ymm6
   %define reg7  ymm7
   %define reg8  ymm8
   %define reg9  ymm9
   %define reg10 ymm10
   %define reg11 ymm11
   %define reg12 ymm12
   %define sizeptr yword
   %define numregs 12
%elif regsize == 512
   %define reg0  zmm0
   %define reg1  zmm1
   %define reg2  zmm2
   %define reg3  zmm3
   %define reg4  zmm4
   %define reg5  zmm5
   %define reg6  zmm6
   %define reg7  zmm7
   %define reg8  zmm8
   %define reg9  zmm9
   %define reg10 zmm10
   %define reg11 zmm11
   %define reg12 zmm12
   %define sizeptr zword      
   %define numregs 12
%elif regsize == 0        ; unspecified size
   %define sizeptr
   %define numregs 0    
%else
   %error unknown register size
%endif

%define modesize 64  ; indicate 64 bit mode

;-----------------------------------------------------------------------------

global TestLoop
global CounterTypesDesired
global NumThreads
global MaxNumCounters
global UsePMC
global PThreadData
global ThreadDataSize
global ClockResultsOS
global PMCResultsOS
global ThreadData
global NumCounters
global Counters
global EventRegistersUsed
global UserData
global RatioOut
global TempOut
global RatioOutTitle
global TempOutTitle

section .note.GNU-stack noalloc noexec nowrite progbits

SECTION .data   align = CACHELINESIZE
default rel


;##############################################################################
;#
;#            List of desired counter types and other user definitions
;#
;##############################################################################
; Here you can select which performance monitor counters you want for your test.
; Select id numbers from the table CounterDefinitions[] in PMCTestA.cpp.

%define USE_PERFORMANCE_COUNTERS   1        ; Tell if you are using performance counters

; Maximum number of PMC counters
%define MAXCOUNTERS   8              ; must match value in PMCTest.h

; Number of PMC counters
%define NUM_COUNTERS  8

CounterTypesDesired:
    DD      counters                 ; macro with desired counter numbers
times (MAXCOUNTERS - ($-CounterTypesDesired)/4)  DD 0

; Number of threads
%define NUM_THREADS   nthreads

; Subtract overhead from clock counts (0 if not)
%define SUBTRACT_OVERHEAD  1

; Number of repetitions in loop to find overhead
%define OVERHEAD_REPETITIONS  4

; Define array sizes
%assign MAXREPEAT  REPETITIONS0


;##############################################################################
;#
;#                       global data
;#
;##############################################################################

; Per-thread data:
align   CACHELINESIZE, DB 0
; Data for first thread
ThreadData:                                                ; beginning of thread data block
CountTemp:     times  (MAXCOUNTERS + 1)          DD   0    ; temporary storage of counts
CountOverhead: times  (MAXCOUNTERS + 1)          DD  -1    ; temporary storage of count overhead
ClockResults:  times   REPETITIONS0              DD   0    ; clock counts
PMCResults:    times  (REPETITIONS0*MAXCOUNTERS)  DD   0    ; PMC counts
align 8, DB 0
RSPSave                                          DQ   0    ; save stack pointer
ALIGN   CACHELINESIZE, DB 0                                ; Make sure threads don't use same cache lines
THREADDSIZE  equ     ($ - ThreadData)                      ; size of data block for each thread

; Define data blocks of same size for remaining threads
%if  NUM_THREADS > 1
  times ((NUM_THREADS-1)*THREADDSIZE)            DB 0
%endif

; Global data
PThreadData     DQ    ThreadData                 ; Pointer to measured data for all threads
NumCounters     DD    0                          ; Will be number of valid counters
MaxNumCounters  DD    NUM_COUNTERS               ; Tell PMCTestA.CPP length of CounterTypesDesired
UsePMC          DD    USE_PERFORMANCE_COUNTERS   ; Tell PMCTestA.CPP if RDPMC used. Driver needed
NumThreads      DD    NUM_THREADS                ; Number of threads
ThreadDataSize  DD    THREADDSIZE                ; Size of each thread data block
ClockResultsOS  DD    ClockResults-ThreadData    ; Offset to ClockResults
PMCResultsOS    DD    PMCResults-ThreadData      ; Offset to PMCResults
Counters:             times MAXCOUNTERS   DD 0   ; Counter register numbers used will be inserted here
EventRegistersUsed    times MAXCOUNTERS   DD 0   ; Set by MTMonA.cpp

%ifmacro extraoutput                            ; define optional extra output columns
   extraoutput
%else
   RatioOut      DD   0, 0, 0, 0                ; optional ratio output. Se PMCTest.h
   TempOut       DD   0                         ; optional arbitrary output. Se PMCTest.h
   RatioOutTitle DQ   0                         ; column heading
   TempOutTitle  DQ   0                         ; column heading
%endif  


;##############################################################################
;#
;#                 User data
;#
;##############################################################################
ALIGN   CACHELINESIZE, DB 0

; Put any data definitions your test code needs here

UserData:
%ifmacro testdata
        testdata
%else
        times 1000H  DB 0
%endif

;##############################################################################
;#
;#                 Macro definitions used in test loop
;#
;##############################################################################

%macro SERIALIZE 0             ; serialize CPU
       xor     eax, eax
       cpuid
%endmacro

%macro CLEARXMMREG 1           ; clear one xmm register
   pxor xmm%1, xmm%1
%endmacro 

%macro CLEARALLXMMREG 0        ; set all xmm or ymm registers to 0
   %if  USEAVX
      VZEROALL                 ; set all ymm registers to 0
   %else
      %assign i 0
      %rep 16
         CLEARXMMREG i         ; set all 16 xmm registers to 0
         %assign i i+1
      %endrep
   %endif
%endmacro


;##############################################################################
;#
;#                    Test Loop
;#
;##############################################################################

SECTION .text   align = codealign
default rel

;extern "C" int TestLoop (int thread) {
; This function runs the code to test REPETITIONS0 times
; and reads the counters before and after each run:

TestLoop:
        push    rbx
        push    rbp
        push    r12
        push    r13
        push    r14
        push    r15
%if     WINDOWS                     ; These registers must be saved in Windows, not in Linux
        push    rsi
        push    rdi
        sub     rsp, 0A8H           ; Space for saving xmm6 - 15 and align
        movaps  [rsp], xmm6
        movaps  [rsp+10H], xmm7
        movaps  [rsp+20H], xmm8
        movaps  [rsp+30H], xmm9
        movaps  [rsp+40H], xmm10
        movaps  [rsp+50H], xmm11
        movaps  [rsp+60H], xmm12
        movaps  [rsp+70H], xmm13
        movaps  [rsp+80H], xmm14
        movaps  [rsp+90H], xmm15        
        mov     r15d, ecx          ; Thread number
%else   ; Linux
        mov     r15d, edi          ; Thread number
%endif
        
; Register use:
;   r13: pointer to thread data block
;   r14: loop counter
;   r15: thread number
;   rax, rbx, rcx, rdx: scratch
;   all other registers: available to user program


;##############################################################################
;#
;#                 Warm up
;#
;##############################################################################
; Get into max frequency state

%if WARMUPCOUNT
        mov ecx, WARMUPCOUNT / 10
        mov eax, 1
        align 16
Warmuploop:
        %rep 10
        imul eax, ecx
        %endrep
        dec ecx
        jnz Warmuploop

%endif

;##############################################################################
;#
;#                 User Initializations 
;#
;##############################################################################
; You may add any initializations your test code needs here.
; Registers esi, edi, ebp and r8 - r12 will be unchanged from here to the 
; Test code start.
; 

        finit                 ; clear all FP registers
        
        CLEARALLXMMREG        ; clear all xmm or ymm registers

        imul eax, r15d, 400h ; separate data for each thread
        lea rsi, [UserData]
        add rsi, rax
        lea rdi, [rsi+200h]
        xor ebp, ebp
        
%define psi rsi              ; esi in 32-bit mode, rsi in 64-bit mode

%ifmacro testinit1
        testinit1
%endif
        

;##############################################################################
;#
;#                 End of user Initializations 
;#
;##############################################################################

        lea     r13, [ThreadData]              ; address of first thread data block
        imul    eax, r15d, THREADDSIZE         ; offset to thread data block
        add     r13, rax                       ; address of current thread data block
        mov     [r13+(RSPSave-ThreadData)],rsp ; save stack pointer

%if  SUBTRACT_OVERHEAD
; First test loop. Measure empty code
        xor     r14d, r14d                     ; Loop counter

TEST_LOOP_1:

        SERIALIZE
      
        ; Read counters
%assign i  0
%rep    NUM_COUNTERS
        mov     ecx, [Counters + i*4]
        rdpmc
        mov     [r13 + i*4 + 4 + (CountTemp-ThreadData)], eax
%assign i  i+1
%endrep
      

        SERIALIZE

        ; read time stamp counter
        rdtsc
        mov     [r13 + (CountTemp-ThreadData)], eax

        SERIALIZE

        ; Empty. Test code goes here in next loop

        SERIALIZE

        ; read time stamp counter
        rdtsc
        sub     [r13 + (CountTemp-ThreadData)], eax        ; CountTemp[0]

        SERIALIZE

        ; Read counters
%assign i  0
%rep    NUM_COUNTERS
        mov     ecx, [Counters + i*4]
        rdpmc
        sub     [r13 + i*4 + 4 + (CountTemp-ThreadData)], eax
%assign i  i+1
%endrep

        SERIALIZE

        ; find minimum counts
%assign i  0
%rep    NUM_COUNTERS + 1
        mov     eax, [r13+i*4+(CountTemp-ThreadData)]       ; -count
        neg     eax
        mov     ebx, [r13+i*4+(CountOverhead-ThreadData)]   ; previous count
        cmp     eax, ebx
        cmovb   ebx, eax
        mov     [r13+i*4+(CountOverhead-ThreadData)], ebx   ; minimum count        
%assign i  i+1
%endrep
        
        ; end second test loop
        inc     r14d
        cmp     r14d, OVERHEAD_REPETITIONS
        jb      TEST_LOOP_1

%endif  ; SUBTRACT_OVERHEAD

        
; Second test loop. Measure user code
        xor     r14d, r14d                    ; Loop counter

TEST_LOOP_2:

%ifmacro testinitc
        testinitc
%endif

        SERIALIZE
      
        ; Read counters
%assign i  0
%rep    NUM_COUNTERS
        mov     ecx, [Counters + i*4]
        rdpmc
        mov     [r13 + i*4 + 4 + (CountTemp-ThreadData)], eax
%assign i  i+1
%endrep

        SERIALIZE

        ; read time stamp counter
        rdtsc
        mov     [r13 + (CountTemp-ThreadData)], eax

        SERIALIZE

;##############################################################################
;#
;#                 Test code start
;#
;##############################################################################

; Put the assembly code to test here
; Don't modify r13, r14, r15!

%ifmacro testinit2
        testinit2
%endif

%if REPETITIONS1 > 1
        mov r12d, REPETITIONS1
        align codealign
REPETITIONS1LOOP:
%endif

%ifmacro testinit3
        testinit3
%endif

%rep REPETITIONS2
        ; test code inserted as macro
        testcode
%endrep

%ifmacro testafter1
        testafter1
%endif

%if REPETITIONS1 > 1
        dec r12d
        jnz REPETITIONS1LOOP
%endif

%ifmacro testafter2
        testafter2
%endif

;##############################################################################
;#
;#                 Test code end
;#
;##############################################################################

        SERIALIZE

        ; read time stamp counter
        rdtsc
        sub     [r13 + (CountTemp-ThreadData)], eax        ; CountTemp[0]

        SERIALIZE

        ; Read counters
%assign i  0
%rep    NUM_COUNTERS
        mov     ecx, [Counters + i*4]
        rdpmc
        sub     [r13 + i*4 + 4 + (CountTemp-ThreadData)], eax  ; CountTemp[i+1]
%assign i  i+1
%endrep

        SERIALIZE

        ; subtract counts before from counts after
        mov     eax, [r13 + (CountTemp-ThreadData)]            ; -count
        neg     eax
%if     SUBTRACT_OVERHEAD
        sub     eax, [r13+(CountOverhead-ThreadData)]   ; overhead clock count        
%endif  ; SUBTRACT_OVERHEAD        
        mov     [r13+r14*4+(ClockResults-ThreadData)], eax      ; save clock count
        
%assign i  0
%rep    NUM_COUNTERS
        mov     eax, [r13 + i*4 + 4 + (CountTemp-ThreadData)]
        neg     eax
%if     SUBTRACT_OVERHEAD
        sub     eax, [r13+i*4+4+(CountOverhead-ThreadData)]   ; overhead pmc count        
%endif  ; SUBTRACT_OVERHEAD        
        mov     [r13+r14*4+i*4*REPETITIONS0+(PMCResults-ThreadData)], eax      ; save count        
%assign i  i+1
%endrep
        
        ; end second test loop
        inc     r14d
        cmp     r14d, REPETITIONS0
        jb      TEST_LOOP_2

        ; clean up
EXITALL1:

%ifmacro testafter3
        testafter3
%endif

        mov     rsp, [r13+(RSPSave-ThreadData)]   ; restore stack pointer        
        finit
        cld
%if  USEAVX
        VZEROALL                       ; clear all ymm registers
%endif

EXITALL2:
        ; return REPETITIONS0;
        mov     eax, REPETITIONS0
        
%if     WINDOWS                        ; Restore registers saved in Windows
        movaps  xmm6, [rsp]
        movaps  xmm7, [rsp+10H]
        movaps  xmm8, [rsp+20H]
        movaps  xmm9, [rsp+30H]
        movaps  xmm10, [rsp+40H]
        movaps  xmm11, [rsp+50H]
        movaps  xmm12, [rsp+60H]
        movaps  xmm13, [rsp+70H]
        movaps  xmm14, [rsp+80H]
        movaps  xmm15, [rsp+90H]
        add     rsp, 0A8H           ; Free space for saving xmm6 - 15
        pop     rdi
        pop     rsi
%endif
        pop     r15
        pop     r14
        pop     r13
        pop     r12
        pop     rbp
        pop     rbx
        ret
        
; End of TestLoop
