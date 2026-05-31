.text
.align 4
.globl  vfmax_gcc16
.type   vfmax_gcc16, @function
vfmax_gcc16:
	movgr2fr.w      $f1,$r4
	movgr2fr.w      $f6,$r6
	bstrpick.d      $r4,$r4,63,32
	fcmp.sgt.s      $fcc0,$f6,$f1
	bstrpick.d      $r13,$r5,63,32
	movgr2fr.w      $f3,$r7
	movgr2fr.w      $f0,$r7
	fmov.s  $f2,$f1
	movgr2fr.w      $f5,$r4
	movgr2fr.w      $f4,$r13
	movgr2fr.w      $f1,$r5
	bstrpick.d      $r6,$r6,63,32
	bstrpick.d      $r7,$r7,63,32
	bceqz   $fcc0,.L33
	movgr2fr.w      $f2,$r6
	fcmp.sgt.s      $fcc0,$f2,$f5
	bceqz   $fcc0,.L34
	movgr2fr.w      $f2,$r7
	slli.w  $r13,$r13,0
	fcmp.sgt.s      $fcc0,$f2,$f4
	slli.w  $r7,$r7,0
	movcf2fr        $f2,$fcc0
	fcmp.sgt.s      $fcc0,$f3,$f1
	movfr2gr.s      $r12,$f2
	bceqz   $fcc0,.L40
	masknez $r13,$r13,$r12
	maskeqz $r12,$r7,$r12
	or      $r12,$r13,$r12
	movgr2fr.w      $f5,$r6
	fmov.s  $f2,$f6
	movgr2fr.w      $f4,$r12
	b       .L9
.L33:
	fcmp.sgt.s      $fcc0,$f3,$f1
	movgr2fr.w      $f6,$r7
	movcf2fr        $f0,$fcc0
	fcmp.sgt.s      $fcc0,$f6,$f4
	movfr2gr.s      $r12,$f0
	movgr2fr.w      $f0,$r6
	bceqz   $fcc0,.L41
	fcmp.sgt.s      $fcc0,$f0,$f5
	movfr2gr.s      $r13,$f1
	movfr2gr.s      $r14,$f3
	bceqz   $fcc0,.L36
	movgr2fr.w      $f5,$r6
.L36:
	movgr2fr.w      $f4,$r7
.L37:
	masknez $r13,$r13,$r12
	maskeqz $r12,$r14,$r12
	or      $r12,$r13,$r12
	movgr2fr.w      $f0,$r12
.L9:
	movfr2gr.d      $r12,$f2
	or      $r4,$r0,$r0
	bstrins.d       $r4,$r12,31,0
	movfr2gr.d      $r12,$f0
	or      $r5,$r0,$r0
	bstrins.d       $r5,$r12,31,0
	movfr2gr.d      $r12,$f5
	bstrins.d       $r4,$r12,63,32
	movfr2gr.d      $r12,$f4
	bstrins.d       $r5,$r12,63,32
	jr      $r1
.L41:
	fcmp.sgt.s      $fcc0,$f0,$f5
	bcnez   $fcc0,.L42
	movfr2gr.s      $r13,$f1
	movfr2gr.s      $r14,$f3
	masknez $r13,$r13,$r12
	maskeqz $r12,$r14,$r12
	or      $r12,$r13,$r12
	movgr2fr.w      $f0,$r12
	b       .L9
.L34:
	fcmp.sgt.s      $fcc0,$f3,$f1
	movgr2fr.w      $f2,$r7
	movfr2gr.s      $r13,$f1
	movcf2fr        $f0,$fcc0
	fcmp.sgt.s      $fcc0,$f2,$f4
	movfr2gr.s      $r12,$f0
	movfr2gr.s      $r14,$f3
	fmov.s  $f2,$f6
	bceqz   $fcc0,.L37
	movgr2fr.w      $f4,$r7
	b       .L37
.L40:
	masknez $r13,$r13,$r12
	maskeqz $r12,$r7,$r12
	or      $r12,$r13,$r12
	movgr2fr.w      $f0,$r5
	movgr2fr.w      $f5,$r6
	fmov.s  $f2,$f6
	movgr2fr.w      $f4,$r12
	b       .L9
.L42:
	movfr2gr.s      $r13,$f1
	movfr2gr.s      $r14,$f3
	masknez $r13,$r13,$r12
	maskeqz $r12,$r14,$r12
	or      $r12,$r13,$r12
	movgr2fr.w      $f5,$r6
	movgr2fr.w      $f0,$r12
	b       .L9
	.size	vfmax_gcc16, .-vfmax_gcc16
