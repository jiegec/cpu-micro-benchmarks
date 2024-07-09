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

%macro testinit3 0
    mov rdi, 1000

    ; loop 300 times to clear phr
    ; since we only consider branch misprediction of the last two branches
    ; we do not have to be accurate here e.g. 93/194
loop_begin:
    mov eax, 300
    align 64
    jmp dummy_target

    align 1<<19
    %rep (1<<19)-(1<<8)
        nop
    %endrep

    ; dummy_target aligned to 1<<8
dummy_target:
    %rep (1<<8)-7
        nop
    %endrep
    dec eax ; 2 bytes
    ; the last byte of jnz aligned to 1<<19
    ; jnz dummy_target
    db 0x0f
    db 0x85
    dd dummy_target - $ - 4

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
    mov eax, 8+1

    align 1<<16
    %rep (1<<16)-(1<<8)
        nop
    %endrep

    ; dummy_target aligned to 1<<8
shift_phr_dummy_target:
    %rep (1<<8)-7
        nop
    %endrep
    dec eax ; 2 bytes
    ; the last byte of jnz aligned to 1<<18
    ; jnz shift_phr_dummy_target
    db 0x0f
    db 0x85
    dd shift_phr_dummy_target - $ - 4

    align 64
    jnz second_target
second_target:

    READ_PMC_END

    align 64
    dec rdi
    jnz loop_begin
%endmacro