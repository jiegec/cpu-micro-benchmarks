CC      := gcc
CFLAGS  := -O2 -march=native -mlasx
LDFLAGS := -lrt

OBJS := transform_c.o transform_gcc16.o transform_gcc15.o transform_clang22.o transform_opt.o benchmark.o

all: benchmark

transform_gcc16.o: transform_gcc16.S
	$(CC) $(CFLAGS) -x assembler-with-cpp -c -o $@ $<

transform_gcc15.o: transform_gcc15.S
	$(CC) $(CFLAGS) -x assembler-with-cpp -c -o $@ $<

transform_clang22.o: transform_clang22.S
	$(CC) $(CFLAGS) -x assembler-with-cpp -c -o $@ $<

transform_opt.o: transform_opt.S
	$(CC) $(CFLAGS) -x assembler-with-cpp -c -o $@ $<

transform_c.o: transform_c.c
	$(CC) $(CFLAGS) -c -o $@ $<

benchmark.o: benchmark.c
	$(CC) $(CFLAGS) -c -o $@ $<

benchmark: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

.PHONY: clean
clean:
	rm -f *.o benchmark
