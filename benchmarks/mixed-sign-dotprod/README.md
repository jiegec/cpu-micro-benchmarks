# mixed-sign-dotprod

Benchmarks of hand-written LASX/LSX assembly dot-product routines against
GCC/Clang auto-vectorized output, for the `transform()` inner loop in Stockfish.

Built for discussion at:
<https://github.com/loongson-community/discussions/issues/119>

## Files

| File | Description |
|------|-------------|
| `transform_c.c` | Reference C implementation |
| `transform_gcc16.S` | LASX (256-bit), from GCC 16 |
| `transform_gcc15.S` | LASX (256-bit), from GCC 15 |
| `transform_clang22.S` | LSX (128-bit), from Clang 22 |
| `transform_opt.S` | Hand-optimized LASX using `xvmulwev.h.bu.b` |
| `benchmark.c` | Validation + timing harness |

## Build

```sh
make
```

## Run

```sh
./benchmark
```
