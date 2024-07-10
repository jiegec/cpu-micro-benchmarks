; reproduce Figure 5 of Half&Half

; alignment bits of branch instruction address
%ifndef branchalign
    %define branchalign 18
%endif

%macro testinit3 0
    mov rdi, 10000

loop_begin:
    ; k = rand() % 2
    rdrand ebx
    and ebx, 1

    ; set phr to k0...0
    ; set lower two bits to 0bk0
    align 1<<5
    %rep (1<<4)-1
        nop
    %endrep
    ; B3=0, B4=1
    jnz zero_target

    align 1<<5
    %rep (1<<5)-1
        nop
    %endrep
    ; B3=0, B4=0
    jz zero_target

    ; T0=T1=0
    align 1<<6
zero_target:

    ; shift phr by 193 times
    ; loop to shift phr
    mov eax, 193+1

    align 1<<16
    %rep (1<<16)-(1<<6)
        nop
    %endrep

    ; dummy_target aligned to 1<<6
shift_phr_dummy_target:
    %rep (1<<6)-7
        nop
    %endrep
    dec eax ; 2 bytes
    ; the last byte of jnz aligned to 1<<16
    ; jnz shift_phr_dummy_target
    db 0x0f
    db 0x85
    dd shift_phr_dummy_target - $ - 4
    
    ; first test branch
    test ebx, ebx
    align 1<<(branchalign+1)
    %rep (1<<branchalign)-1
        nop
    %endrep
    jnz first_target
first_target:

    ; set phr to k0...0 again
    test ebx, ebx
    
    ; set lower two bits to 0bk0
    align 1<<5
    %rep (1<<4)-1
        nop
    %endrep
    ; B3=0, B4=1
    jnz zero_target_2
    
    align 1<<5
    %rep (1<<5)-1
        nop
    %endrep
    ; B3=0, B4=0
    jz zero_target_2

    ; T0=T1=0
    align 1<<6
zero_target_2:

    ; shift phr by 193 times
    ; loop to shift phr
    mov eax, 193+1

    align 1<<16
    %rep (1<<16)-(1<<6)
        nop
    %endrep

    ; dummy_target aligned to 1<<6
shift_phr_dummy_target_2:
    %rep (1<<6)-7
        nop
    %endrep
    dec eax ; 2 bytes
    ; the last byte of jnz aligned to 1<<16
    ; jnz shift_phr_dummy_target_2
    db 0x0f
    db 0x85
    dd shift_phr_dummy_target_2 - $ - 4

    READ_PMC_START
    ; second test branch
    ; in the opposite direction
    test ebx, ebx ; 2 bytes
    align 1<<(branchalign+1)
    %rep (1<<branchalign)-1
        nop
    %endrep
    jz second_target
second_target:

    READ_PMC_END

    align 64
    dec rdi
    jnz loop_begin
    
%endmacro