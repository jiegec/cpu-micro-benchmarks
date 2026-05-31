# Optimized LASX (256-bit) implementation of transform()
# Strategy: use xvmulwev.h.bu.b / xvmulwod.h.bu.b for signed*unsigned widening
# multiply, then xvhaddw.w.h to horizontally accumulate to int32_t.
# Arguments: a0=output(int32_t*), a1=weights(int8_t*), a2=biases(int32_t*), a3=input(uint8_t*)

.text
.globl  transform_opt
.type   transform_opt, @function
transform_opt:
        or      $r13, $r5, $r0          # r13 = weights (advancing pointer)
        or      $r15, $r0, $r0          # r15 = i * 4 (byte offset into output/biases)
        lu12i.w $r14, 1                 # r14 = 0x1000 (inner loop bound = 4096 bytes)
        lu12i.w $r17, 4                 # r17 = 0x4000 (outer loop bound = 4096 * 4)
.L3_opt:
        ldx.w   $r16, $r6, $r15         # r16 = biases[i]
        xvldi   $xr7, 0                 # xr7 = 0 (accumulator for 8 word lanes)
        or      $r12, $r0, $r0          # r12 = j (byte offset into weights/input)
.L2_opt:
        xvldx   $xr3, $r13, $r12        # xr3 = weights[32 bytes]
        xvldx   $xr0, $r7, $r12         # xr0 = input[32 bytes]

        # Widening multiply: unsigned input × signed weights → halfwords
        xvmulwev.h.bu.b $xr1, $xr0, $xr3   # even bytes: 16 halfword products
        xvmulwod.h.bu.b $xr2, $xr0, $xr3   # odd bytes: 16 halfword products

        # Horizontal accumulate: adjacent halfword pairs → words
        xvhaddw.w.h $xr4, $xr1, $xr1       # 8 words from even products
        xvhaddw.w.h $xr5, $xr2, $xr2       # 8 words from odd products

        # Combine even + odd contributions
        xvadd.w $xr5, $xr4, $xr5

        # Accumulate into running sum
        xvadd.w $xr7, $xr5, $xr7

        addi.d  $r12, $r12, 32
        bne     $r12, $r14, .L2_opt

        # Reduce 8 word lanes to a single int32_t
        xvhaddw.d.w     $xr7, $xr7, $xr7   # 4 doublewords
        add.d   $r13, $r13, $r12            # advance weights by 0x1000
        xvhaddw.q.d     $xr0, $xr7, $xr7   # 2 quadwords
        xvpermi.d       $xr7, $xr0, 2      # swap halves
        xvadd.d $xr7, $xr7, $xr0           # final sum
        xvpickve2gr.w   $r12, $xr7, 0      # extract to GPR

        add.w   $r12, $r12, $r16           # sum += bias
        stx.w   $r12, $r4, $r15            # output[i] = sum

        addi.d  $r15, $r15, 4
        bne     $r15, $r17, .L3_opt
        jr      $r1
.size   transform_opt, .-transform_opt
