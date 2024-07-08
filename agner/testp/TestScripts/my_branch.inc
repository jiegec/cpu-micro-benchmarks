; alignment bits of branch instruction address
%ifndef branchalign
    %define branchalign 18
%endif

; alignment bits of branch target address
%ifndef targetalign
    %define targetalign 5
%endif

; number of branches tracked in phr
%define phrbranches 200

%macro testinit3 0
    mov rdi, 1000

    ; loop phrbranches+1 times to clear phr
loop_begin:
    mov eax, phrbranches+1
    align 64
    jmp dummy_target

    align 1<<19
    %rep (1<<19)-(1<<8)
        nop
    %endrep

    ; dummy_target aligned to 1<<8
dummy_target:
    %rep (1<<8)-8
        nop
    %endrep
    dec eax
    ; jnz aligned to 1<<19
    jnz dummy_target

    READ_PMC_START
    rdrand eax
    and eax, 1

    ; READ_PMC_START: 166
    ; rdrand eax: 3 bytes
    ; and eax, 1: 3 bytes
    ; jnz first_target: 2 or 6 bytes depending on targetalign
    %if targetalign >= 7
        %assign jnzlength 6
    %else
        %assign jnzlength 2
    %endif

    %rep (1<<branchalign)-166-6-jnzlength
        nop
    %endrep

    jnz first_target

    %rep (1<<targetalign)
        nop
    %endrep
first_target:

    ; set PC[5]=1
    align 64
    nop
    align 32
    jnz second_target
second_target:

    READ_PMC_END

    align 64
    dec rdi
    jnz loop_begin
%endmacro