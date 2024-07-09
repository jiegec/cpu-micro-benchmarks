; number of dummy branches
%ifndef dummybranches
    %define dummybranches 200
%endif

%macro testinit3 0
    mov rdi, 1000
    READ_PMC_START

loop_begin:

    ; train branch
    rdrand eax
    and eax, 1
    jnz first_target
first_target:

    ; dummy branches
    %assign i 0
    %rep dummybranches
    jmp dummy_branch_%+ i
dummy_branch_%+ i:
    %assign i i+1
    %endrep

    ; test branch
    test eax, eax
    jnz second_target
second_target:

    dec rdi
    jnz loop_begin

    READ_PMC_END


%endmacro