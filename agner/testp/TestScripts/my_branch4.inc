; reproduce Figure 4 of Half&Half
; additionally shift PHR by 16 bits to avoid tag collision

; alignment bits of branch instruction address
%ifndef branchalign
    %define branchalign 18
%endif

; alignment bits of branch target address
%ifndef targetalign
    %define targetalign 5
%endif

; toggle bit of branch address
%ifndef branchtoggle
    %define branchtoggle 0
%endif

; toggle bit of target address
%ifndef targettoggle
    %define targettoggle 0
%endif

%macro SHIFT_PHR 1
    mov eax, %1+1

    align 1<<16
    %rep (1<<16)-(1<<6)
        nop
    %endrep

    ; dummy_target aligned to 1<<6
%%shift_phr_dummy_target:
    %rep (1<<6)-7
        nop
    %endrep
    dec eax ; 2 bytes
    ; the last byte of jnz aligned to 1<<16
    ; jnz shift_phr_dummy_target
    db 0x0f
    db 0x85
    dd %%shift_phr_dummy_target - $ - 4
%endmacro

%macro testinit3 0
    mov rdi, 1000

loop_begin:
    ; loop 300 times to clear phr
    SHIFT_PHR 300

    READ_PMC_START
    rdrand eax
    and eax, 1

    ; READ_PMC_START: 166
    ; rdrand eax: 3 bytes
    ; and eax, 1: 3 bytes
    ; jnz first_target: 6 bytes

    %rep (1<<branchalign)-166-6-6
        nop
    %endrep

    %rep (1<<branchtoggle)
        nop
    %endrep

    ; the last byte of jnz minus 1<<branchtoggle aligned to 1<<branchalign
    ; jnz first_target
    db 0x0f
    db 0x85
    dd first_target - $ - 4

    %rep (1<<targetalign)-1-(1<<branchtoggle)
        nop
    %endrep
    %rep (1<<targettoggle)
        nop
    %endrep
    ; target minus 1<<targettoggle aligned to 1<<targetalign
first_target:

    ; loop to shift phr by 16 bits in 8 loops
    SHIFT_PHR 8

    align 64
    jnz second_target
second_target:

    READ_PMC_END

    align 64
    dec rdi
    jnz loop_begin
%endmacro