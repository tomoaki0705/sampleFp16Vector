
#if defined(__arm__) || defined(_M_ARM) || defined(__aarch64__)
#include <arm_neon.h>
typedef float32x4_t float4;
typedef float16x4_t half4;
typedef uint8x8_t uchar8;
typedef uint16x8_t ushort8;
typedef uint32x4_t uint4; 
inline uchar8 load_uchar8_copy(uint8_t c)   { return vdup_n_u8(c); }
inline float4 load_float4_copy(float32_t f) { return vdupq_n_f32(f); }
inline float4 load_float4(const float* ptr) { return *(float4*)ptr; }
inline half4 load_half4(const short* ptr)   { return *(float16x4_t*)ptr; }
inline uchar8 load_uchar8(const unsigned char* ptr) { return vld1_u8(ptr); }
inline uchar8 load_uchar8_repeat(unsigned char c)   { return vdup_n_u8(c); };
inline float4 set_float4(float f3, float f2, float f1, float f0) { float32x4_t a = vdupq_n_f32(f0);a = vsetq_lane_f32(f1, a, 1);a = vsetq_lane_f32(f2, a, 2);a = vsetq_lane_f32(f3, a, 3); return a; }
inline uchar8 set_uchar8(char c7, char c6, char c5, char c4, char c3, char c2, char c1, char c0) { return  vcreate_u8(((uint64_t)c0) << 56 | ((uint64_t)c1) << 48 | ((uint64_t)c2) << 40 | ((uint64_t)c3) << 32 | ((uint64_t)c4) << 24 | ((uint64_t)c5) << 16 | ((uint64_t)c6) << 8 | c7); };
inline float get_float(float4 f, int i)     { return vgetq_lane_f32(f, i); }
inline short get_half(half4 h, int i)       { return vgetq_lane_s16(h, i); }
inline void store_half4(void* ptr, half4 h) { *(half4*)ptr = h; }
inline void store_uchar8(void* ptr, uchar8 c)   { vst1_u8((unsigned char*)ptr, c); }
inline void store_uint4(void* ptr, uint4 i) { vst1q_u32((unsigned int*)ptr, i); } 
inline half4 convert_float4_half4(float4 f) { return vcvt_f16_f32(f); }
inline ushort8 convert_uchar8_ushort8(uchar8 c) { return vmovl_u8(c); }
inline uint4 convert_ushort8_lo_uint4(ushort8 s){ return vmovl_u16(vget_low_s16 (s)); }
inline uint4 convert_ushort8_hi_uint4(ushort8 s){ return vmovl_u16(vget_high_s16(s)); }
inline uchar8 convert_ushort8_uchar8(ushort8 s) { return vmovn_u16(s); }
inline float4 convert_uint4_float4(uint4 i) { return vcvtq_f32_u32(i); }
inline float4 convert_half4_float4(half4 h) { return vcvt_f32_f16(h); }
inline uint4 convert_float4_uint4(float4 f) { return vcvtq_u32_f32(vaddq_f32(f,vdupq_n_f32(0.5f))); }
inline ushort8 convert_uint4_ushort8(uint4 a, uint4 b){ return vcombine_u16(vmovn_u16(a), vmovn_u16(b)); }
inline float4 multiply_float4(float4 a, float4 b)     { return vmulq_f32(a, b); }
#elif defined(__x86_64__) || defined(_M_X64) || defined(_M_IX86) || defined(i386)
#include <immintrin.h>
typedef __m128 float4;
typedef __m128i half4;
typedef __m128i uchar8;
typedef __m128i ushort8;
typedef __m128i uint4; 
inline float4 load_float4(const float* ptr) { return _mm_load_ps(ptr); }
inline half4 load_half4(const short* ptr)   { return _mm_loadl_epi64((const half4*)ptr); }
inline float4 load_float4_copy(float f)     { return _mm_set1_ps(f); }
inline uchar8 load_uchar8(const unsigned char* ptr) { return _mm_loadl_epi64((const uchar8*)ptr); }
inline uchar8 load_uchar8_repeat(unsigned char c)   { return _mm_set1_epi8(c); };
inline float4 set_float4(float f3, float f2, float f1, float f0) { return _mm_set_ps(f3, f2, f1, f0); }
inline uchar8 set_uchar8(char c7, char c6, char c5, char c4, char c3, char c2, char c1, char c0) { return _mm_set_epi8 (0, 0, 0, 0, 0, 0, 0, 0, c7, c6, c5, c4, c3, c2, c1, c0); };
inline float get_float(float4 f, int i)     { return _mm_extract_ps(f, i); }
inline short get_half(half4 h, int i)       { return _mm_extract_epi16(h, i); }
inline void store_half4(void* ptr, half4 h) { _mm_storel_epi64((half4*)ptr, h); }
inline void store_uchar8(void* ptr, uchar8 c)   { _mm_storel_epi64((uchar8*)ptr, c); }
inline void store_uint4(void* ptr, uint4 i) { _mm_store_si128((uint4*)ptr, i); } 
inline half4 convert_float4_half4(float4 f) { return _mm_cvtps_ph(f, 0); }
inline ushort8 convert_uchar8_ushort8(uchar8 c) { return _mm_cvtepu8_epi16(c); }
inline uint4 convert_ushort8_lo_uint4(ushort8 s){ __m128i v_zero = _mm_setzero_si128(); return _mm_unpacklo_epi16(s, v_zero); }
inline uint4 convert_ushort8_hi_uint4(ushort8 s){ __m128i v_zero = _mm_setzero_si128(); return _mm_unpackhi_epi16(s, v_zero); }
inline uchar8 convert_ushort8_uchar8(ushort8 s) { __m128i v_zero = _mm_setzero_si128(); return _mm_packus_epi16(s, v_zero); }
inline float4 convert_uint4_float4(uint4 i) { return _mm_cvtepi32_ps(i); }
inline float4 convert_half4_float4(half4 h) { return _mm_cvtph_ps(h); }
inline uint4 convert_float4_uint4(float4 f) { return _mm_cvtps_epi32(f); }
inline ushort8 convert_uint4_ushort8(uint4 a, uint4 b){ return _mm_packs_epi32(a, b); }
inline float4 multiply_float4(float4 a, float4 b)     { return _mm_mul_ps(a, b); }
#ifdef _MSC_VER
inline ushort8 operator += (uchar8 a, uchar8 b)     { return _mm_add_epi8(a, b); }
inline ushort8 operator + (uchar8 a, uchar8 b)      { return _mm_add_epi8(a, b); }
#include <intrin.h>
#elif defined (__GNUC__)
#include <cpuid.h>
#endif
#endif


