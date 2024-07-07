; 32bitinstr.inc
; Define test code for instructions in 32-bit mode 
; (c) 2012 by Agner Fog. GNU General Public License www.gnu.org/licenses

; instruction-specific test codes

%ifidni instruct, aam_latency
   %macro testcode 0
      aam
   %endmacro

%elifidni instruct, aam_throughput
   %macro testcode 0
      xor eax,eax   ; break dependency
      aam
   %endmacro

%elifidni instruct, bound
   %macro testinit2 0
      mov [esi],esi        ; setup bounds to avoid interrupt
      lea eax, [esi+100]
      mov [esi+4],eax
   %endmacro
   %macro testcode 0
      bound esi, [esi]
   %endmacro

%elifidni instruct, into
   %macro testinit2 0
      xor eax,eax   ; clear overflow flag
   %endmacro

%elifidni instruct, lahf_sahf
   %macro testcode 0
      lahf            ; test combined latency
      sahf
   %endmacro

%elifidni instruct, leave
   %macro testcode 0
      mov esi, 100 ; can't use ebp here
      align 16
      repeat11loop:
      mov edi,esp          ; prepare stack frame
      push 0
      mov ebp,esp
      mov [ebp],ebp
      %rep 100
         leave
      %endrep
      mov esp,edi          ; restore stack
      dec esi
      jnz repeat11loop     ; loop
   %endmacro
   %define repeat1 0       ; disable default loops
   %define repeat2 1

%elifidni instruct, pushad
   %macro testcode 0
      mov edi,esp
      %rep 100
         pushad
      %endrep
      mov esp,edi          ; restore stack
   %endmacro
   %define repeat2 1

%elifidni instruct, popad
   %macro testcode 0
      movd xmm0, esp
      movd xmm1, ebp
      sub esp, 3200        ; prepare stack
      %rep 100
         popad
      %endrep
      movd esp, xmm0       ; restore stack
      movd ebp, xmm1       ; restore loop pointer
   %endmacro
   %define repeat2 1

%elifidni instruct, salc_inc_al
   %macro testcode 0
      salc                 ; combined latency
      inc al
   %endmacro

%elifidni instruct, enter
   %macro testcode 0
      mov esi, 100          ; can't use ebp here
      align 16
      repeat11loop:
      mov edi, esp
      lea ebp, [UserData+1000h]   ; dummy frame
      %REP 100
      enter 4, immvalue
      %ENDREP
      mov esp,edi                 ; restore stack pointer
      dec esi
      jnz repeat11loop            ; loop 
   %endmacro
   %define repeat1 0       ; disable default loops
   %define repeat2 1


%else


%endif

