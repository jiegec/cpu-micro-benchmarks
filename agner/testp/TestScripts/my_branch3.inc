; reproduce Table 2 of Half&Half

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

; number of dummy branches
%ifndef dummybranches
    %define dummybranches 5
%endif

%macro testinit3 0
    mov rdi, 1000

loop_begin:

    ; loop to clear phr
    mov eax, 200
    align 64
    jmp clear_phr_dummy_target

    align 1<<16
    %rep (1<<16)-(1<<8)
        nop
    %endrep

    ; dummy_target aligned to 1<<8
clear_phr_dummy_target:
    %rep (1<<8)-7
        nop
    %endrep
    dec eax ; 2 bytes
    ; the last byte of jnz aligned to 1<<18
    ; jnz clear_phr_dummy_target
    db 0x0f
    db 0x85
    dd clear_phr_dummy_target - $ - 4

    ; train branch

    READ_PMC_START
    rdrand ebx
    and ebx, 1
    ; READ_PMC_START: 166 bytes
    ; rdrand ebx: 3 bytes
    ; and ebx, 1: 3 bytes
    ; jnz first_target: 6 bytes
    %rep (1<<branchalign)-166-6-6
        nop
    %endrep

    %rep (1<<branchtoggle)
        nop
    %endrep
    ; the last byte of jnz - 1<<branchtoggle aligned to 1<<branchalign
    ; jnz first_target
    db 0x0f
    db 0x85
    dd first_target - $ - 4

    ; target aligned to 1<<targetalign
    %rep (1<<targetalign)-1-(1<<branchtoggle)
        nop
    %endrep
first_target:

    ; loop to shift phr
    mov eax, dummybranches+1

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

    ; test branch

    align 64
    and ebx, 1
    jnz second_target
second_target:
    READ_PMC_END

    align 64
    dec rdi
    jnz loop_begin

%endmacro