%macro testinit3 0
    mov rdi, 1000

loop_begin:

    mov eax, 195
    align 64
    jmp dummy_target

    align 1<<19
    %rep (1<<19)-(1<<8)
    nop
    %endrep

dummy_target:
    %rep (1<<8)-8
    nop
    %endrep
    dec eax
    jnz dummy_target

    rdrand eax
    and eax, 1

    %rep (1<<16)-6-2
    nop
    %endrep
    jnz first_target

    %rep (1<<6)
    nop
    %endrep
first_target:

    align 64
    nop
    align 32
    jnz second_target
second_target:
    align 64
    dec rdi
    jnz loop_begin
%endmacro