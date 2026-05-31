.text
.globl  vfmax_opt
.type   vfmax_opt, @function
.align  5
vfmax_opt:
    vinsgr2vr.d $vr0, $a0, 0     // vr0[0:63]   = a.m[0], a.m[1]
    vinsgr2vr.d $vr0, $a1, 1     // vr0[64:127] = a.m[2], a.m[3]
    vinsgr2vr.d $vr1, $a2, 0     // vr1[0:63]   = b.m[0], b.m[1]
    vinsgr2vr.d $vr1, $a3, 1     // vr1[64:127] = b.m[2], b.m[3]
    vfcmp.slt.s $vr2, $vr0, $vr1       // vr2 = all-1s where a < b (equiv to b > a, IEEE 754)
    vbitsel.v   $vr0, $vr0, $vr1, $vr2 // va=1->vk(b), va=0->vj(a): max = b>a ? b : a
    vpickve2gr.d $a0, $vr0, 0    // $a0 = lower 64 bits of result
    vpickve2gr.d $a1, $vr0, 1    // $a1 = upper 64 bits of result
    jr $r1
.size   vfmax_opt, .-vfmax_opt
