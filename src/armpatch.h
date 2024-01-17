// This just patches up the ancient code so it builds in 241 arm
#if defined(__arm__) || defined(__aarch64__)
#define ARM_NEON 1
#else
#include <immintrin.h>
#endif
