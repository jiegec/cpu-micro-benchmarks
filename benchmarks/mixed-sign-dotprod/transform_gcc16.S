# LASX (256-bit vector) — originally from GCC 16
# Arguments: a0=output(int32_t*), a1=weights(int8_t*), a2=biases(int32_t*), a3=input(uint8_t*)

.text
.globl  transform_gcc16
.type   transform_gcc16, @function
transform_gcc16:
        or      $r13,$r5,$r0
        xvldi   $xr7,128
        xvldi   $xr6,64
        or      $r15,$r0,$r0
        lu12i.w $r14,1                    # 0x1000 = 4096
        lu12i.w $r17,4                    # 0x4000 = 16384 = 4096 * 4
.L3_lasx:
        ldx.w   $r16,$r6,$r15
        xvldi   $xr5,0
        or      $r12,$r0,$r0
.L2_lasx:
        xvldx   $xr3,$r13,$r12
        xvldx   $xr1,$r7,$r12
        xvmulwod.h.b    $xr2,$xr3,$xr6
        xvmulwev.h.b    $xr0,$xr3,$xr6
        xvhaddw.w.h     $xr4,$xr2,$xr2
        xvhaddw.w.h     $xr0,$xr0,$xr0
        addi.d  $r12,$r12,32
        xvadd.w $xr0,$xr0,$xr4
        xvadd.w $xr5,$xr0,$xr5
        xvadd.b $xr1,$xr1,$xr7
        xvadd.w $xr0,$xr0,$xr5
        xvmulwev.h.b    $xr2,$xr1,$xr3
        xvmulwod.h.b    $xr1,$xr1,$xr3
        xvhaddw.w.h     $xr2,$xr2,$xr2
        xvhaddw.w.h     $xr1,$xr1,$xr1
        xvadd.w $xr2,$xr2,$xr1
        xvadd.w $xr5,$xr2,$xr0
        bne     $r12,$r14,.L2_lasx
        xvhaddw.d.w     $xr5,$xr5,$xr5
        add.d   $r13,$r13,$r12
        xvhaddw.q.d     $xr0,$xr5,$xr5
        xvpermi.d       $xr5,$xr0,2
        xvadd.d $xr5,$xr5,$xr0
        xvpickve2gr.w   $r12,$xr5,0
        add.w   $r12,$r12,$r16
        stx.w   $r12,$r4,$r15
        addi.d  $r15,$r15,4
        bne     $r15,$r17,.L3_lasx
        jr      $r1
.size   transform_gcc16, .-transform_gcc16
