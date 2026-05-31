CC      := gcc
CFLAGS  := -O2 -march=native -mlsx

OBJS := vfmax_c.o vfmax_gcc16.o vfmax_clang22.o vfmax_opt.o benchmark.o

all: benchmark

vfmax_gcc16.o: vfmax_gcc16.S
	$(CC) $(CFLAGS) -x assembler-with-cpp -c -o $@ $<

vfmax_clang22.o: vfmax_clang22.S
	$(CC) $(CFLAGS) -x assembler-with-cpp -c -o $@ $<

vfmax_opt.o: vfmax_opt.S
	$(CC) $(CFLAGS) -x assembler-with-cpp -c -o $@ $<

vfmax_c.o: vfmax_c.c
	$(CC) $(CFLAGS) -c -o $@ $<

benchmark.o: benchmark.c
	$(CC) $(CFLAGS) -c -o $@ $<

benchmark: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: clean
clean:
	rm -f *.o benchmark
