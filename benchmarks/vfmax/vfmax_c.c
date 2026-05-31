#include <stdint.h>

struct vfloat4 {
  float m[4];
};

struct vmask4 {
  int32_t m[4];
};

static struct vmask4 op_gt(struct vfloat4 a, struct vfloat4 b) {
  struct vmask4 r;
  r.m[0] = a.m[0] > b.m[0] ? -1 : 0;
  r.m[1] = a.m[1] > b.m[1] ? -1 : 0;
  r.m[2] = a.m[2] > b.m[2] ? -1 : 0;
  r.m[3] = a.m[3] > b.m[3] ? -1 : 0;
  return r;
}

static struct vfloat4 select(struct vfloat4 a, struct vfloat4 b,
                             struct vmask4 cond) {
  struct vfloat4 r;
  r.m[0] = (cond.m[0] & (int32_t)0x80000000) ? b.m[0] : a.m[0];
  r.m[1] = (cond.m[1] & (int32_t)0x80000000) ? b.m[1] : a.m[1];
  r.m[2] = (cond.m[2] & (int32_t)0x80000000) ? b.m[2] : a.m[2];
  r.m[3] = (cond.m[3] & (int32_t)0x80000000) ? b.m[3] : a.m[3];
  return r;
}

struct vfloat4 vfmax_c(struct vfloat4 a, struct vfloat4 b) {
  struct vmask4 mask = op_gt(b, a);
  return select(a, b, mask);
}
