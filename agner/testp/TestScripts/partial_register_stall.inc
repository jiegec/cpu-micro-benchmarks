; partial_register_stall.inc                               2022-11-02 Agner Fog

; Test if partial register access causes stall, synchronization uops,
; or false dependencies
;
; (c) 2013-2022 by Agner Fog. GNU General Public License www.gnu.org/licenses
;
; Parameters:
;
; tcase:       Test case:
;              1:   al, ax      (reading modified and unmodified parts)
;              2:   ah, eax     (reading modified and unmodified parts)
;              3:   bx, ebx     (reading modified and unmodified parts)
;              4:   al, ah      (possible false dependence)
;              5:   inc, jc     (reading unmodified bit)
;              6:   inc, jbe    (reading modified and unmodified bits)
;              7:   inc, cmovbe (reading modified and unmodified bits)
;              8:   inc, pushf  (reading modified and unmodified bits)
;              9:   sahf, jg    (reading modified and unmodified bits)
;             10:   bt, cmovbe  (reading modified and unmodified bits)
;             11:   shr, setbe  (reading modified bits)
;             12:   movss, orps (reading modified and unmodified parts)
;             13:   movlps,orps (reading modified and unmodified parts)
;             14:   xmm, ymm    (VEX <-> non-VEX transition)
;             15:   xmm, ymm    (reading modified and unmodified parts)
;             16:   VEC vs. non-VEX (reading unmodified parts)

; tmode:       1:   Test the code with partial dependence that may cause stall
;              2:   Comparison code without partial dependence

; main testcode

%macro testcode 0

%if tcase == 1     ; al, ax      (reading modified and unmodified parts)
   
   %rep 100
      %if tmode == 1
         inc al
         inc ax   
      %else
         inc al
         inc al
      %endif
   %endrep   

%elif tcase == 2     ; ah, eax      (reading modified and unmodified parts)
   
   %rep 100
      %if tmode == 1
         inc ah
         inc eax   
      %else
         inc eax
         inc eax
      %endif
   %endrep   
   
%elif tcase == 3   ; bx, ebx     (reading modified and unmodified parts)

   %rep 100
      %if tmode == 1
         inc bx
         inc ebx   
      %else
         inc ebx   
         inc ebx   
      %endif
   %endrep   

%elif tcase == 4   ; al, ah      (possible false dependence)

   %rep 100
      %if tmode == 1
         inc al
         inc ah   
      %else
         inc al
         inc bh
      %endif
   %endrep   

%elif tcase == 5   ; inc, jc     (reading unmodified bit)

   %rep 100
      %if tmode == 1
         inc eax
         jc $+3
         nop
      %else
         inc eax
         jz $+3
         nop
      %endif
   %endrep
   
%elif tcase == 6   ; inc, jbe    (reading modified and unmodified bits)

   mov ebx, 1
   %rep 100
      %if tmode == 1
         inc eax
         jbe $+3
         nop
      %else
         add eax, ebx
         jbe $+3
         nop
      %endif
   %endrep
   
%elif tcase == 7   ; inc, cmovbe (reading modified and unmodified bits)

   %rep 100
      %if tmode == 1
         inc eax
         cmovbe eax,ecx
      %else
         add eax, ebx
         cmovbe eax,ecx
      %endif
   %endrep

%elif tcase == 8   ; inc, pushf  (reading modified and unmodified bits)

   %rep 100
      %if tmode == 1
         inc rax
         pushfq
         pop rax
      %else
         add rax, rbx
         pushfq
         pop rax
      %endif
   %endrep

%elif tcase == 9   ; sahf, setg    (reading modified and unmodified bits)

   %rep 100
      %if tmode == 1
         sahf
         setg ah
      %else
         sahf
         setz ah
      %endif
   %endrep

%elif tcase == 10  ; bt, cmovbe  (reading modified and unmodified bits)

   %rep 100
      %if tmode == 1
         bt eax,2
         cmovbe eax,ebx
      %else
         bt eax,2
         cmovb eax,ebx
      %endif
   %endrep

%elif tcase == 11  ; shr, setbe  (reading modified bits)

   %rep 100
      %if tmode == 1
         shr eax,5
         cmovbe eax,ebx
      %else
         add eax,5
         cmovbe eax,ebx
      %endif
   %endrep
   
%elif tcase == 12  ; ror, setc  (reading modified bits)

   %rep 100
      %if tmode == 1
         ror al,5
         setc al
      %else
         add al,5
         setc al
      %endif
   %endrep

%elif tcase == 13  ; movss, orps (reading modified and unmodified parts)

   %rep 100
      %if tmode == 1
         movss xmm1, xmm2
         orps  xmm1, xmm3
      %else
         movaps xmm1, xmm1
         orps  xmm1, xmm3
      %endif
   %endrep
   
%elif tcase == 14  ; movlps, orps (reading modified and unmodified parts)

   %rep 100
      %if tmode == 1
         movlps xmm1, [rsi]
         orps  xmm1, xmm3
      %else
         orps xmm1, [rsi]
         orps  xmm1, xmm3
      %endif
   %endrep

%elif tcase == 15  ; xmm, ymm    (reading modified and unmodified parts)

   %rep 100
      %if tmode == 1
         orps xmm1, xmm2
         vorps ymm1, ymm3
      %else
         vorps xmm1, xmm2
         vorps ymm1, ymm3
      %endif
   %endrep

%elif tcase == 16  ; VEC vs. non-VEX    (reading unmodified parts)

 ;  vpcmpeqw ymm0,ymm0,ymm0
 ;  vpcmpeqw ymm1,ymm1,ymm1
   vpcmpeqw ymm6,ymm6,ymm6
   
   %rep 100
      %if tmode == 1
         movaps xmm1, xmm0
         mulps xmm1, xmm2
      %else
         vmovaps xmm1, xmm0
         vmulps xmm1, xmm1, xmm2
      %endif
   %endrep

%else 
  
    %error unknown test mode tcase
    
%endif

%endmacro

; test loops
%define repeat1 1000
%define repeat2 1
