.text
.align 4
.globl  vfmax_gcc15
.type   vfmax_gcc15, @function
vfmax_gcc15:
    addi.d  $r3,$r3,-16
    st.w    $r6,$r3,12
    st.w    $r4,$r3,4
    fld.s   $f0,$r3,12
    fld.s   $f1,$r3,4
    bstrpick.d      $r6,$r6,63,32
    bstrpick.d      $r13,$r7,63,32
    fcmp.sgt.s      $fcc0,$f0,$f1
    bstrpick.d      $r4,$r4,63,32
    bstrpick.d      $r12,$r5,63,32
    stptr.w $r7,$r3,0
    st.w    $r5,$r3,8
    movgr2fr.w      $f2,$r6
    movgr2fr.w      $f3,$r13
    movgr2fr.w      $f0,$r4
    movgr2fr.w      $f1,$r12
    bceqz   $fcc0,.L41
    fcmp.sgt.s      $fcc0,$f2,$f0
    bceqz   $fcc0,.L42
    fcmp.sgt.s      $fcc0,$f3,$f1
    fld.s   $f4,$r3,0
    fld.s   $f5,$r3,8
    movcf2fr        $f0,$fcc0
    movfr2gr.s      $r12,$f0
    fcmp.sgt.s      $fcc0,$f4,$f5
    bcnez   $fcc0,.L6
    beqz    $r12,.L8
    fmov.s  $f1,$f3
.L8:
    fld.s   $f0,$r3,8
    fst.s   $f0,$r3,0
    fmov.s  $f0,$f2
    fld.s   $f2,$r3,12
    fst.s   $f2,$r3,4
.L9:
    ld.w    $r15,$r3,4
    ldptr.w $r14,$r3,0
    movfr2gr.s      $r12,$f0
    movfr2gr.s      $r13,$f1
    or      $r4,$r0,$r0
    or      $r5,$r0,$r0
    bstrins.d       $r4,$r15,31,0
    bstrins.d       $r5,$r14,31,0
    bstrins.d       $r4,$r12,63,32
    bstrins.d       $r5,$r13,63,32
    addi.d  $r3,$r3,16
    jr      $r1
.L41:
    fld.s   $f4,$r3,0
    fld.s   $f5,$r3,8
    fcmp.sgt.s      $fcc0,$f4,$f5
    movcf2fr        $f4,$fcc0
    fcmp.sgt.s      $fcc0,$f3,$f1
    movfr2gr.s      $r12,$f4
    bceqz   $fcc0,.L47
    fcmp.sgt.s      $fcc0,$f2,$f0
    bceqz   $fcc0,.L44
    fmov.s  $f0,$f5
    bnez    $r12,.L48
.L27:
    fst.s   $f0,$r3,0
    fmov.s  $f1,$f3
    fmov.s  $f0,$f2
    b       .L9
.L47:
    fcmp.sgt.s      $fcc0,$f2,$f0
    bcnez   $fcc0,.L49
    fmov.s  $f2,$f5
    beqz    $r12,.L21
    fld.s   $f2,$r3,0
.L21:
    fst.s   $f2,$r3,0
    b       .L9
.L42:
    fld.s   $f2,$r3,0
    fld.s   $f4,$r3,8
    fcmp.sgt.s      $fcc0,$f2,$f4
    movcf2fr        $f2,$fcc0
    fcmp.sgt.s      $fcc0,$f3,$f1
    movfr2gr.s      $r12,$f2
    bcnez   $fcc0,.L15
    fmov.s  $f2,$f4
    beqz    $r12,.L17
    fld.s   $f2,$r3,0
.L17:
    fst.s   $f2,$r3,0
    fld.s   $f2,$r3,12
    fst.s   $f2,$r3,4
    b       .L9
.L6:
    beqz    $r12,.L11
    fmov.s  $f1,$f3
.L11:
    fmov.s  $f0,$f2
    fld.s   $f2,$r3,12
    fst.s   $f2,$r3,4
    b       .L9
.L15:
    fmov.s  $f1,$f4
    bnez    $r12,.L50
.L23:
    fst.s   $f1,$r3,0
    fld.s   $f1,$r3,12
    fst.s   $f1,$r3,4
    fmov.s  $f1,$f3
    b       .L9
.L49:
    fmov.s  $f0,$f5
    beqz    $r12,.L19
    fld.s   $f0,$r3,0
.L19:
    fst.s   $f0,$r3,0
    fmov.s  $f0,$f2
    b       .L9
.L44:
    fmov.s  $f1,$f5
    beqz    $r12,.L29
    fld.s   $f1,$r3,0
.L29:
    fst.s   $f1,$r3,0
    fmov.s  $f1,$f3
    b       .L9
.L48:
    fld.s   $f0,$r3,0
    b       .L27
.L50:
    fld.s   $f1,$r3,0
    b       .L23
	.size	vfmax_gcc15, .-vfmax_gcc15
