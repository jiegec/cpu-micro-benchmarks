# LASX (256-bit vector) — originally from GCC 15
# Arguments: a0=output(int32_t*), a1=weights(int8_t*), a2=biases(int32_t*), a3=input(uint8_t*)

.text
.globl  transform_gcc15
.type   transform_gcc15, @function
transform_gcc15:
        or      $r13,$r5,$r0
        or      $r15,$r0,$r0
        lu12i.w $r14,1                    # 0x1
        lu12i.w $r17,4                    # 0x4
.L3_gcc15:
        ldx.w   $r16,$r6,$r15
        xvrepli.b       $xr2,0
        or      $r12,$r0,$r0
.L2_gcc15:
        xvldx   $xr3,$r13,$r12
        xvldx   $xr0,$r7,$r12
        vext2xv.h.b     $xr8,$xr3
        vext2xv.hu.bu   $xr1,$xr0
        xvpermi.q       $xr5,$xr3,1
        xvmul.h $xr1,$xr1,$xr8
        xvpermi.q       $xr4,$xr0,1
        vext2xv.w.h     $xr3,$xr1
        xvpermi.q       $xr6,$xr1,1
        xvadd.w $xr3,$xr3,$xr2
        vext2xv.hu.bu   $xr0,$xr4
        vext2xv.w.h     $xr2,$xr6
        vext2xv.h.b     $xr8,$xr5
        addi.d  $r12,$r12,32
        xvmul.h $xr0,$xr0,$xr8
        vext2xv.w.h     $xr1,$xr0
        xvpermi.q       $xr7,$xr0,1
        xvadd.w $xr0,$xr2,$xr3
        vext2xv.w.h     $xr2,$xr7
        xvadd.w $xr0,$xr1,$xr0
        xvadd.w $xr2,$xr2,$xr0
        bne     $r12,$r14,.L2_gcc15
        xvhaddw.d.w     $xr2,$xr2,$xr2
        add.d   $r13,$r13,$r12
        xvhaddw.q.d     $xr0,$xr2,$xr2
        xvpermi.d       $xr2,$xr0,2
        xvadd.d $xr2,$xr2,$xr0
        xvpickve2gr.w   $r12,$xr2,0
        add.w   $r12,$r12,$r16
        stx.w   $r12,$r4,$r15
        addi.d  $r15,$r15,4
        bne     $r15,$r17,.L3_gcc15
        jr      $r1
.size   transform_gcc15, .-transform_gcc15
