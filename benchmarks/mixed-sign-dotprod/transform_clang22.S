# LSX (128-bit vector) — originally from Clang 22
# Arguments: a0=output(int32_t*), a1=weights(int8_t*), a2=biases(int32_t*), a3=input(uint8_t*)

.text
.globl  transform_clang22
.type   transform_clang22, @function
transform_clang22:
        move    $a4, $zero
        vrepli.b        $vr0, 0
        lu12i.w $a5, -1
        lu12i.w $a6, 1
.LBB0_1_lsx:
        slli.d  $a7, $a4, 2
        ldx.w   $t0, $a2, $a7
        vori.b  $vr1, $vr0, 0
        vinsgr2vr.w     $vr1, $t0, 0
        move    $t0, $a5
.LBB0_2_lsx:
        add.d   $t1, $a1, $t0
        ldx.w   $t1, $t1, $a6
        vinsgr2vr.w     $vr2, $t1, 0
        vilvl.b $vr2, $vr2, $vr2
        add.d   $t1, $a3, $t0
        ldx.w   $t1, $t1, $a6
        vilvl.h $vr2, $vr2, $vr2
        vslli.w $vr2, $vr2, 24
        vsrai.w $vr2, $vr2, 24
        vinsgr2vr.w     $vr3, $t1, 0
        vilvl.b $vr3, $vr0, $vr3
        vilvl.h $vr3, $vr0, $vr3
        addi.d  $t0, $t0, 4
        vmadd.w $vr1, $vr3, $vr2
        bnez    $t0, .LBB0_2_lsx
        vhaddw.d.w      $vr1, $vr1, $vr1
        vhaddw.q.d      $vr1, $vr1, $vr1
        vpickve2gr.d    $t0, $vr1, 0
        stx.w   $t0, $a0, $a7
        addi.d  $a4, $a4, 1
        add.d   $a1, $a1, $a6
        bne     $a4, $a6, .LBB0_1_lsx
        ret
.size   transform_clang22, .-transform_clang22
