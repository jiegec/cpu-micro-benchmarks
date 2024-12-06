#ifndef __JIT_H__
#define __JIT_H__

#include "include/utils.h"
#include <assert.h>
#include <cstdint>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

class jit {
private:
  // mmap-ed base
  uint8_t *base;

  // current address
  uint8_t *cur;

  // mmaped memory region size
  size_t size;

public:
  // mmap at random base
  jit(size_t size) {
    cur = base = (uint8_t *)mmap(NULL, size, PROT_READ | PROT_WRITE,
                                 MAP_ANON | MAP_PRIVATE, -1, 0);
    assert(base != MAP_FAILED);
    this->size = size;
  }

  // mmap at fixed base
  jit(void *fixed_base, size_t size) {
    cur = base = (uint8_t *)mmap(fixed_base, size, PROT_READ | PROT_WRITE,
                                 MAP_ANON | MAP_PRIVATE | MAP_FIXED, -1, 0);
    assert(base != MAP_FAILED);
    this->size = size;
  }

  ~jit() { munmap(base, size); }

  uint8_t *get_cur() const { return cur; }

  void set_cur(uint8_t *new_cur) { cur = new_cur; }

  // align with custom offset
  // after: (cur - offset) % alignment == 0
  uint8_t *align(size_t alignment, ssize_t offset) {
    size_t addr = (size_t)cur;
    ssize_t mod = addr % alignment;
    if (offset >= 0) {
      if (mod <= offset) {
        addr += mod - offset;
      } else {
        addr += alignment - mod + offset;
      }
    } else {
      if (mod - offset <= (ssize_t)alignment) {
        addr += alignment - (mod - offset);
      } else {
        addr += alignment + alignment + offset - mod;
      }
    }

    uint8_t *new_cur = (uint8_t *)addr;
    assert((size_t)cur <= (size_t)new_cur);

    // fill with nop
#if defined(__x86_64__)
    uint8_t *p = cur;
    while (p < (uint8_t *)new_cur) {
      *p++ = 0x90;
    }
#elif defined(__aarch64__)
    assert((size_t)cur % 4 == 0);
    assert((size_t)new_cur % 4 == 0);
    uint32_t *p = (uint32_t *)cur;
    while (p < (uint32_t *)new_cur) {
      *p++ = 0xd503201f;
    }
#endif

    cur = (uint8_t *)addr;
    assert(((size_t)cur - offset) % alignment == 0);
    return cur;
  }

  void protect() { assert(mprotect(base, size, PROT_READ | PROT_EXEC) == 0); }

#if defined(__aarch64__)

  void nop() {
    assert((size_t)cur % 4 == 0);
    uint32_t *p = (uint32_t *)cur;
    *p = 0xd503201f;
    cur += 4;
  }

  void svc0() {
    assert((size_t)cur % 4 == 0);
    uint32_t *p = (uint32_t *)cur;
    *p = 0xd4000001;
    cur += 4;
  }

  void ret() {
    assert((size_t)cur % 4 == 0);
    uint32_t *p = (uint32_t *)cur;
    *p = 0xd65f03c0;
    cur += 4;
  }

  // SUB <Xd|SP>, <Xn|SP>, #<imm>{, <shift>}
  void sub64(int rd, int rn, int imm) {
    assert((size_t)cur % 4 == 0);
    uint32_t *p = (uint32_t *)cur;
    *p = 0xd1000000;
    assert((0 <= rd) && (rd <= 31));
    *p |= rd;
    assert((0 <= rn) && (rn <= 31));
    *p |= (rn << 5);
    assert((0 <= imm) && (imm <= (1 << 12) - 1));
    *p |= (imm << 10);
    cur += 4;
  }

  // ADD <Xd|SP>, <Xn|SP>, #<imm>{, <shift>}
  void add64(int rd, int rn, int imm) {
    assert((size_t)cur % 4 == 0);
    uint32_t *p = (uint32_t *)cur;
    *p = 0x91000000;
    assert((0 <= rd) && (rd <= 31));
    *p |= rd;
    assert((0 <= rn) && (rn <= 31));
    *p |= (rn << 5);
    assert((0 <= imm) && (imm <= (1 << 12) - 1));
    *p |= (imm << 10);
    cur += 4;
  }

  // ADD <Xd>, <Xn>, <Xm>{, <shift> #<amount>}
  void addr64(int rd, int rn, int rm) {
    assert((size_t)cur % 4 == 0);
    uint32_t *p = (uint32_t *)cur;
    *p = 0x8b000000;
    assert((0 <= rd) && (rd <= 31));
    *p |= rd;
    assert((0 <= rn) && (rn <= 31));
    *p |= (rn << 5);
    assert((0 <= rm) && (rm <= 31));
    *p |= (rm << 16);
    cur += 4;
  }

  // SUBS <Xd|SP>, <Xn|SP>, #<imm>{, <shift>}
  void subs64(int rd, int rn, int imm) {
    assert((size_t)cur % 4 == 0);
    uint32_t *p = (uint32_t *)cur;
    *p = 0xf1000000;
    assert((0 <= rd) && (rd <= 31));
    *p |= rd;
    assert((0 <= rn) && (rn <= 31));
    *p |= (rn << 5);
    assert((0 <= imm) && (imm <= (1 << 12) - 1));
    *p |= (imm << 10);
    cur += 4;
  }

  // CMP <Wn|WSP>, #<imm>{, <shift>}
  void cmp32(int rn, int imm) {
    assert((size_t)cur % 4 == 0);
    uint32_t *p = (uint32_t *)cur;
    *p = 0x7100001f;
    assert((0 <= rn) && (rn <= 31));
    *p |= (rn << 5);
    assert((0 <= imm) && (imm <= (1 << 12) - 1));
    *p |= (imm << 10);
    cur += 4;
  }

  // AND <Xd>, <Xn>, <Xm>
  void and64(int rd, int rn, int rm) {
    assert((size_t)cur % 4 == 0);
    uint32_t *p = (uint32_t *)cur;
    *p = 0x8a000000;
    assert((0 <= rd) && (rd <= 31));
    *p |= rd;
    assert((0 <= rn) && (rn <= 31));
    *p |= (rn << 5);
    assert((0 <= rm) && (rm <= 31));
    *p |= (rm << 16);
    cur += 4;
  }

  // EOR <Xd>, <Xn>, <Xm>
  void eor64(int rd, int rn, int rm) {
    assert((size_t)cur % 4 == 0);
    uint32_t *p = (uint32_t *)cur;
    *p = 0xca000000;
    assert((0 <= rd) && (rd <= 31));
    *p |= rd;
    assert((0 <= rn) && (rn <= 31));
    *p |= (rn << 5);
    assert((0 <= rm) && (rm <= 31));
    *p |= (rm << 16);
    cur += 4;
  }

  // CBNZ <Xt>, <label>
  void cbnz64(int rt, uint8_t *target) {
    assert((size_t)cur % 4 == 0);
    uint32_t *p = (uint32_t *)cur;
    *p = 0xb5000000;
    assert((0 <= rt) && (rt <= 31));
    *p |= rt;
    ssize_t imm = ((ssize_t)target - (ssize_t)cur);
    assert(imm < (1 << 20));
    assert(imm >= -(1 << 20));
    assert((imm & 3) == 0);
    *p |= (((size_t)imm & 0x1fffff) >> 2) << 5;
    cur += 4;
  }

  // CBZ <Wt>, <label>
  void cbz32(int rt, uint8_t *target) {
    assert((size_t)cur % 4 == 0);
    uint32_t *p = (uint32_t *)cur;
    *p = 0x34000000;
    assert((0 <= rt) && (rt <= 31));
    *p |= rt;
    ssize_t imm = ((ssize_t)target - (ssize_t)cur);
    assert(imm < (1 << 21));
    assert(imm >= -(1 << 21));
    assert((imm & 3) == 0);
    *p |= (((size_t)imm & 0x1fffff) >> 2) << 5;
    cur += 4;
  }

  // B <label>
  void b(uint8_t *target) {
    assert((size_t)cur % 4 == 0);
    uint32_t *p = (uint32_t *)cur;
    *p = 0x14000000;
    ssize_t imm = ((ssize_t)target - (ssize_t)cur);
    assert(imm < (1 << 28));
    assert(imm >= -(1 << 28));
    assert((imm & 3) == 0);
    *p |= (((size_t)imm & 0xfffffff) >> 2);
    cur += 4;
  }

  // STP <Xt1>, <Xt2>, [<Xn|SP>, #<imm>]
  void stp64(int rt1, int rt2, int rn, int imm) {
    assert((size_t)cur % 4 == 0);
    uint32_t *p = (uint32_t *)cur;
    *p = 0xa9000000;
    assert((0 <= rt1) && (rt1 <= 31));
    *p |= rt1;
    assert((0 <= rn) && (rn <= 31));
    *p |= (rn << 5);
    assert((0 <= rt2) && (rt2 <= 31));
    *p |= (rt2 << 10);
    assert(imm % 8 == 0);
    imm /= 8;
    assert((0 <= imm) && (imm <= (1 << 7) - 1));
    *p |= (imm << 15);
    cur += 4;
  }

  // LDP <Xt1>, <Xt2>, [<Xn|SP>{, #<imm>}]
  void ldp64(int rt1, int rt2, int rn, int imm) {
    assert((size_t)cur % 4 == 0);
    uint32_t *p = (uint32_t *)cur;
    *p = 0xa9400000;
    assert((0 <= rt1) && (rt1 <= 31));
    *p |= rt1;
    assert((0 <= rn) && (rn <= 31));
    *p |= (rn << 5);
    assert((0 <= rt2) && (rt2 <= 31));
    *p |= (rt2 << 10);
    assert(imm % 8 == 0);
    imm /= 8;
    assert((0 <= imm) && (imm <= (1 << 7) - 1));
    *p |= (imm << 15);
    cur += 4;
  }

  // LDR <Wt>, [<Xn|SP>, (<Wm>|<Xm>){, <extend> {<amount>}}]
  void ldr32shift2(int rt, int rn, int rm) {
    assert((size_t)cur % 4 == 0);
    uint32_t *p = (uint32_t *)cur;
    *p = 0xb8605800;
    assert((0 <= rt) && (rt <= 31));
    *p |= rt;
    assert((0 <= rn) && (rn <= 31));
    *p |= (rn << 5);
    assert((0 <= rm) && (rm <= 31));
    *p |= (rm << 16);
    cur += 4;
  }

  // LDR <Xt>, [<Xn|SP>, (<Wm>|<Xm>){, <extend> {<amount>}}]
  void ldr64shift3(int rt, int rn, int rm) {
    assert((size_t)cur % 4 == 0);
    uint32_t *p = (uint32_t *)cur;
    *p = 0xf8607800;
    assert((0 <= rt) && (rt <= 31));
    *p |= rt;
    assert((0 <= rn) && (rn <= 31));
    *p |= (rn << 5);
    assert((0 <= rm) && (rm <= 31));
    *p |= (rm << 16);
    cur += 4;
  }

  // MOV <Xd>, #<imm>
  void mov64(int rd, int imm) {
    assert((size_t)cur % 4 == 0);
    uint32_t *p = (uint32_t *)cur;
    *p = 0xd2800000;
    assert((0 <= rd) && (rd <= 31));
    *p |= rd;
    assert((0 <= imm) && (imm <= (1 << 16) - 1));
    *p |= (imm << 5);
    cur += 4;
  }

  // MOVK <Xd>, #<imm>{, LSL #<shift>}
  void movk64(int rd, int imm, int shift) {
    assert((size_t)cur % 4 == 0);
    uint32_t *p = (uint32_t *)cur;
    *p = 0xf2800000;
    assert((0 <= rd) && (rd <= 31));
    *p |= rd;
    assert((0 <= imm) && (imm <= (1 << 16) - 1));
    *p |= (imm << 5);
    assert((shift % 16) == 0);
    // encoded in the "hw" field as <shift>/16.
    assert((0 <= shift) && (shift <= 48));
    *p |= (shift / 16) << 21;
    cur += 4;
  }

  // MOV <Xd>, #<imm>
  // 3x MOVK <Xd>, #<imm>{, LSL #<shift>}
  void li64(int rd, uint64_t imm) {
    mov64(rd, imm & 0xFFFF);
    movk64(rd, (imm >> 16) & 0xFFFF, 16);
    movk64(rd, (imm >> 32) & 0xFFFF, 32);
    movk64(rd, (imm >> 48) & 0xFFFF, 48);
  }

  // Condition codes
  enum {
    COND_EQ = 0b0000,
    COND_NE = 0b0001,
  };

  // CSEL <Xd>, <Xn>, <Xm>, <cond>
  void csel64(int rd, int rn, int rm, int cond) {
    assert((size_t)cur % 4 == 0);
    uint32_t *p = (uint32_t *)cur;
    *p = 0x9a800000;
    assert((0 <= rd) && (rd <= 31));
    *p |= rd;
    assert((0 <= rn) && (rn <= 31));
    *p |= (rn << 5);
    assert((0 <= rm) && (rm <= 31));
    *p |= (rm << 16);
    assert((0 <= cond) && (cond <= (1 << 4) - 1));
    *p |= (cond << 12);
    cur += 4;
  }

  // BR <Xn>
  void br64(int rd) {
    assert((size_t)cur % 4 == 0);
    uint32_t *p = (uint32_t *)cur;
    *p = 0xd61f0000;
    assert((0 <= rd) && (rd <= 31));
    *p |= (rd << 5);
    cur += 4;
  }

  // BLR <Xn>
  void blr64(int rd) {
    assert((size_t)cur % 4 == 0);
    uint32_t *p = (uint32_t *)cur;
    *p = 0xd63f0000;
    assert((0 <= rd) && (rd <= 31));
    *p |= (rd << 5);
    cur += 4;
  }

  // LSR <Xd>, <Xn>, #<shift>
  void lsr64(int rd, int rn, int shift) {
    assert((size_t)cur % 4 == 0);
    uint32_t *p = (uint32_t *)cur;
    *p = 0xd340fc00;
    assert((0 <= rd) && (rd <= 31));
    *p |= rd;
    assert((0 <= rn) && (rn <= 31));
    *p |= (rn << 5);
    assert((0 <= shift) && (shift <= 63));
    *p |= (shift << 16);
    cur += 4;
  }

  // LSL <Xd>, <Xn>, #<shift>
  void lsl64(int rd, int rn, int shift) {
    assert((size_t)cur % 4 == 0);
    uint32_t *p = (uint32_t *)cur;
    *p = 0xd3400000;
    assert((0 <= rd) && (rd <= 31));
    *p |= rd;
    assert((0 <= rn) && (rn <= 31));
    *p |= (rn << 5);
    assert((1 <= shift) && (shift <= 63));
    // immr
    int immr = 64 - shift;
    *p |= (immr << 16);
    // imms
    *p |= ((immr - 1) << 10);
    cur += 4;
  }

  // UDIV <Xd>, <Xn>, <Xm>
  void udiv64(int rd, int rn, int rm) {
    assert((size_t)cur % 4 == 0);
    uint32_t *p = (uint32_t *)cur;
    *p = 0x9ac00800;
    assert((0 <= rd) && (rd <= 31));
    *p |= rd;
    assert((0 <= rn) && (rn <= 31));
    *p |= (rn << 5);
    assert((0 <= rm) && (rm <= 31));
    *p |= (rm << 16);
    cur += 4;
  }

  // MSUB <Xd>, <Xn>, <Xm>, <Xa>
  void msub64(int rd, int rn, int rm, int ra) {
    assert((size_t)cur % 4 == 0);
    uint32_t *p = (uint32_t *)cur;
    *p = 0x9b008000;
    assert((0 <= rd) && (rd <= 31));
    *p |= rd;
    assert((0 <= rn) && (rn <= 31));
    *p |= (rn << 5);
    assert((0 <= ra) && (ra <= 31));
    *p |= (ra << 10);
    assert((0 <= rm) && (rm <= 31));
    *p |= (rm << 16);
    cur += 4;
  }
#elif defined(HOST_AMD64)
  // register
  enum {
    AX = 0b0000,
    CX = 0b0001,
    DX = 0b0010,
    BX = 0b0011,
    SI = 0b0110,
    DI = 0b0111,
  };

  // dec r64
  void dec_r64(int r) {
    // REX.W + FF /1  DEC r/m64
    uint8_t *p = (uint8_t *)cur;
    *p++ = 0x48;
    *p++ = 0xff;
    *p++ = 0xc8 + r;
    cur += 3;
  }

  // dec r32
  void dec_r32(int r) {
    // FF /1  DEC r/m32
    uint8_t *p = (uint8_t *)cur;
    *p++ = 0xff;
    *p++ = 0xc8 + r;
    cur += 2;
  }

  // return
  void ret() {
    uint8_t *p = (uint8_t *)cur;
    *p++ = 0xc3;
    cur = p;
  }

  // one byte nop
  void nop1() {
    uint8_t *p = (uint8_t *)cur;
    *p++ = 0x90;
    cur = p;
  }

  // two byte nop
  void nop2() {
    uint8_t *p = (uint8_t *)cur;
    *p++ = 0x66;
    *p++ = 0x90;
    cur = p;
  }

  // three byte nop
  void nop3() {
    uint8_t *p = (uint8_t *)cur;
    *p++ = 0x0f;
    *p++ = 0x1f;
    *p++ = 0x00;
    cur = p;
  }

  // 5-byte jmp
  void jmp5(uint8_t *target) {
    uint8_t *p = (uint8_t *)cur;
    // E9 cd JMP rel32
    *p++ = 0xe9;
    uint64_t address = (uint64_t)target;
    uint64_t offset = address - (uint64_t)p - 4;
    uint32_t saved_offset = offset;
    assert((uint64_t)(int64_t)(int32_t)saved_offset == offset);
    *p++ = (saved_offset >> 0) & 0xFF;
    *p++ = (saved_offset >> 8) & 0xFF;
    *p++ = (saved_offset >> 16) & 0xFF;
    *p++ = (saved_offset >> 24) & 0xFF;
    cur = p;
  }

  // 2-byte jmp
  void jmp2(uint8_t *target) {
    uint8_t *p = (uint8_t *)cur;
    // EB cb
    *p++ = 0xeb;
    uint64_t address = (uint64_t)target;
    uint64_t offset = address - (uint64_t)p - 1;
    uint32_t saved_offset = offset;
    assert((uint64_t)(int64_t)(int8_t)saved_offset == offset);
    *p++ = (saved_offset >> 0) & 0xFF;
    cur = p;
  }

  // 2-byte jnz
  void jnz2(uint8_t *target) {
    uint8_t *p = (uint8_t *)cur;
    // 75 cb JNZ rel8
    *p++ = 0x75;
    uint64_t address = (uint64_t)target;
    uint64_t offset = address - (uint64_t)p - 1;
    uint32_t saved_offset = offset;
    assert((uint64_t)(int64_t)(int8_t)saved_offset == offset);
    *p++ = (saved_offset >> 0) & 0xFF;
    cur = p;
  }

  // 2-byte jz
  void jz2(uint8_t *target) {
    uint8_t *p = (uint8_t *)cur;
    // 74 cb 	JZ rel8
    *p++ = 0x74;
    uint64_t address = (uint64_t)target;
    uint64_t offset = address - (uint64_t)p - 1;
    uint32_t saved_offset = offset;
    assert((uint64_t)(int64_t)(int8_t)saved_offset == offset);
    *p++ = (saved_offset >> 0) & 0xFF;
    cur = p;
  }

  // 6-byte jnz
  void jnz6(uint8_t *target) {
    uint8_t *p = (uint8_t *)cur;
    // 0F 85 cd JNZ rel32
    *p++ = 0x0f;
    *p++ = 0x85;
    uint64_t address = (uint64_t)target;
    uint64_t offset = address - (uint64_t)p - 4;
    uint32_t saved_offset = offset;
    assert((uint64_t)(int64_t)(int32_t)saved_offset == offset);
    *p++ = (saved_offset >> 0) & 0xFF;
    *p++ = (saved_offset >> 8) & 0xFF;
    *p++ = (saved_offset >> 16) & 0xFF;
    *p++ = (saved_offset >> 24) & 0xFF;
    cur = p;
  }

  // 6-byte jz
  void jz6(uint8_t *target) {
    uint8_t *p = (uint8_t *)cur;
    // 0F 84 cd JZ rel32
    *p++ = 0x0f;
    *p++ = 0x84;
    uint64_t address = (uint64_t)target;
    uint64_t offset = address - (uint64_t)p - 4;
    uint32_t saved_offset = offset;
    assert((uint64_t)(int64_t)(int32_t)saved_offset == offset);
    *p++ = (saved_offset >> 0) & 0xFF;
    *p++ = (saved_offset >> 8) & 0xFF;
    *p++ = (saved_offset >> 16) & 0xFF;
    *p++ = (saved_offset >> 24) & 0xFF;
    cur = p;
  }

  // 8-byte movabs
  void movabs8(int reg, uint64_t imm) {
    uint8_t *p = (uint8_t *)cur;
    // REX.W + B8 + rd io  MOV r64, imm64
    *p++ = 0x48;
    *p++ = 0xb8 + reg;
    *p++ = (imm >> 0) & 0xFF;
    *p++ = (imm >> 8) & 0xFF;
    *p++ = (imm >> 16) & 0xFF;
    *p++ = (imm >> 24) & 0xFF;
    *p++ = (imm >> 32) & 0xFF;
    *p++ = (imm >> 40) & 0xFF;
    *p++ = (imm >> 48) & 0xFF;
    *p++ = (imm >> 56) & 0xFF;
    cur = p;
  }

  // 2-byte jmp register
  void jmpr64(int reg) {
    uint8_t *p = (uint8_t *)cur;
    // FF /4 JMP r/m64
    *p++ = 0xff;
    *p++ = 0xe0 + reg;
    cur = p;
  }

  // 2-byte push 64-bit register
  void push(int reg) {
    uint8_t *p = (uint8_t *)cur;
    // 50+rd PUSH r64
    *p++ = 0x50 + reg;
    cur = p;
  }

  // 2-byte pop 64-bit register
  void pop(int reg) {
    uint8_t *p = (uint8_t *)cur;
    // 58+rd PUSH r64
    *p++ = 0x58 + reg;
    cur = p;
  }

  // 2-byte test 32-bit register
  void test32(int r1, int r2) {
    uint8_t *p = (uint8_t *)cur;
    // 85 /r 	TEST r/m32, r32
    *p++ = 0x85;
    *p++ = 0xc0 | (r2 << 3) | r1;
    cur = p;
  }

  // 3-byte load 32 bit from memory
  // e.g. mov ebx, [rsi+rdi*4]
  void load32_shift2(int dest, int base, int index) {
    uint8_t *p = (uint8_t *)cur;
    // 8B /r MOV r32, r/m32
    *p++ = 0x8b;
    // ebx
    *p++ = 0x4 | (dest << 3);
    // [base+index*4]
    *p++ = 0x80 | base | (index << 3);
    cur = p;
  }

  // 5-byte load 32 bit imm
  // e.g. mov eax, 200
  void movi32(int dest, uint32_t value) {
    uint8_t *p = (uint8_t *)cur;
    // B8+ rd id MOV r32, imm32
    *p++ = 0xb8 + dest;
    *p++ = (value >> 0) & 0xFF;
    *p++ = (value >> 8) & 0xFF;
    *p++ = (value >> 16) & 0xFF;
    *p++ = (value >> 24) & 0xFF;
    cur = p;
  }

  // 4-byte conditional move if zero
  // e.g. cmovz rcx, rdx
  void cmovz64(int dest, int src) {
    uint8_t *p = (uint8_t *)cur;
    // REX.W + 0F 44 /r 	CMOVZ r64, r/m64
    *p++ = 0x48;
    *p++ = 0x0f;
    *p++ = 0x44;
    *p++ = 0xc0 | (dest << 3) | src;
    cur = p;
  }
#endif

  void dump(const char *file_name = "jit.bin") const {
    // objdump -m aarch64 -b binary -D jit.bin
    FILE *fp = fopen(file_name, "wb");
    fwrite(base, 1, cur - base, fp);
    fclose(fp);
  }
};

#endif