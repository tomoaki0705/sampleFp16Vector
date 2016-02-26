
#if defined(__arm__) || defined(_M_ARM)
#include <arm_neon.h>
typedef float32x4_t float4;
typedef float16x4_t half4;
inline float4 load_float4(const float* ptr) { return *(float4*)ptr; }
inline void store_half4(void* ptr, half4 h) { *(half4*)ptr = h; }
inline half4 convert_float4_half4(float4 f) { return vcvt_f16_f32(f); }
#elif defined(__x86_64__) || defined(_M_X64) || defined(_M_IX86)
#include <immintrin.h>
typedef __m128 float4;
typedef __m128i half4;
inline float4 load_float4(const float* ptr) { return _mm_load_ps(ptr); }
inline void store_half4(void* ptr, half4 h) { _mm_storel_epi64((half4*)ptr, h); }
inline half4 convert_float4_half4(float4 f) { return _mm_cvtps_ph(f, 0); }
#endif
