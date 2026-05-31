.text
.align 4
.globl	vfmax_clang22
.type	vfmax_clang22,@function
vfmax_clang22:
	movgr2fr.w      $fa0, $a2
	movgr2fr.w      $fa1, $a3
	movgr2fr.w      $fa2, $a0
	movgr2fr.w      $fa3, $a1
	fcmp.cule.s     $fcc1, $fa0, $fa2
	fcmp.cule.s     $fcc0, $fa1, $fa3
	vinsgr2vr.d     $vr0, $a3, 0
	vinsgr2vr.d     $vr0, $a2, 1
	vsrli.d $vr0, $vr0, 32
	vshuf4i.w       $vr0, $vr0, 8
	vinsgr2vr.d     $vr1, $a1, 0
	vinsgr2vr.d     $vr1, $a0, 1
	vsrli.d $vr1, $vr1, 32
	vshuf4i.w       $vr1, $vr1, 8
	vfcmp.clt.s     $vr0, $vr1, $vr0
	vshuf4i.w       $vr1, $vr0, 16
	movcf2gr        $a4, $fcc1
	masknez $a5, $a2, $a4
	maskeqz $a4, $a0, $a4
	or      $a4, $a4, $a5
	vpickve2gr.d    $a5, $vr1, 1
	andi    $a5, $a5, 1
	masknez $a0, $a0, $a5
	maskeqz $a2, $a2, $a5
	or      $a0, $a2, $a0
	movcf2gr        $a2, $fcc0
	masknez $a5, $a3, $a2
	maskeqz $a2, $a1, $a2
	or      $a2, $a2, $a5
	vpickve2gr.d    $a5, $vr0, 0
	andi    $a5, $a5, 1
	masknez $a1, $a1, $a5
	maskeqz $a3, $a3, $a5
	or      $a1, $a3, $a1
	bstrpick.d      $a3, $a4, 31, 0
	bstrins.d       $a0, $a3, 31, 0
	bstrpick.d      $a2, $a2, 31, 0
	bstrins.d       $a1, $a2, 31, 0
	ret
	.size	vfmax_clang22, .-vfmax_clang22
