#ifndef Y11__UTILS_H
#define Y11__UTILS_H

#include <stdlib.h>
#include <string.h>

static inline void* copy(const void* p, size_t s){
  void* r = malloc(s);
  if(!r) return 0;
  memcpy(r, p, s);
  return r;
}

#if __STDC_VERSION__ >= 202311L
#define u_typeof typeof
#else
#define u_typeof __typeof__
#endif

#define tcopy(...) ((u_typeof(__VA_ARGS__)*) copy(&(__VA_ARGS__), sizeof(__VA_ARGS__)))

#endif
