
#if defined(__arm__) || defined(_M_ARM)
#include <arm_neon.h>
#elif defined(__x86_64__) || defined(_M_X64) || defined(_M_IX86)
#include <immintrin.h>
#endif
#include "floatMul.h"

void float2half(float* src, short* dst, int length) {
#if defined(__arm__) || defined(_M_ARM)
	// requires NEON
	const unsigned int cParallel = 4;
	for (int i = 0; i <= length-cParallel; i+=cParallel)
	{
		float32x4_t float_vector = *(float32x4_t*)(src + i);
		float16x4_t half_vector = vcvt_f16_f32( float_vector );
		*(float16x4_t*)(dst + i) = half_vector;
	}
#elif defined(__x86_64__) || defined(_M_X64) || defined(_M_IX86)
	// requires AVX
	const unsigned int cParallel = 8;
	for (int i = 0; i <= length-cParallel; i+=cParallel)
	{
		__m256 float_vector = _mm256_load_ps(src + i);
		__m128i half_vector = _mm256_cvtps_ph(float_vector, 0);
		*(__m128i*)(dst + i) = half_vector;
	}
#endif

}

void multiply_float(unsigned char* src, float* gain, unsigned char* dst, unsigned int cSize)
{
	for (int i = 0;i < cSize;i++)
	{
		dst[i] = (unsigned char)(src[i] * gain[i]);
	}
}

void multiply(unsigned char* src, short* gain, unsigned char* dst, unsigned int cSize)
{
#if defined(__arm__) || defined(_M_ARM)
	for (int x = 0;x <= cSize - 8;x += 8)
	{
		// load 64bits
		uint8x8_t srcInteger = vld1_u8(src+x);
		// load 32bits
		float16x4_t gainHalfLow  = *(float16x4_t*)(gain + x     );
		float16x4_t gainHalfHigh = *(float16x4_t*)(gain + x + 4 );

		// uchar -> ushort
		uint16x8_t srcIntegerShort  = vmovl_u8(srcInteger);
		
		// ushort -> uint
		uint32x4_t srcIntegerLow  = vmovl_u16(vget_low_s16 (srcIntegerShort));
		uint32x4_t srcIntegerHigh = vmovl_u16(vget_high_s16(srcIntegerShort));

		// uint -> float
		float32x4_t srcFloatLow  = vcvtq_f32_u32(srcIntegerLow );
		float32x4_t srcFloatHigh = vcvtq_f32_u32(srcIntegerHigh);

		// half -> float
		float32x4_t gainFloatLow  = vcvt_f32_f16(gainHalfLow );
		float32x4_t gainFloatHigh = vcvt_f32_f16(gainHalfHigh);

		// float * float (per elment)
		float32x4_t dstFloatLow  = vmulq_f32(srcFloatLow,  gainFloatLow );
		float32x4_t dstFloatHigh = vmulq_f32(srcFloatHigh, gainFloatHigh);

		// float -> uint
		uint32x4_t dstIntegerLow  = vcvtq_u32_f32(dstFloatLow );
		uint32x4_t dstIntegerHigh = vcvtq_u32_f32(dstFloatHigh);

		// uint -> ushort
		uint16x8_t dstIntegerShort = vcombine_u16(vmovn_u16(dstIntegerLow),  vmovn_u16(dstIntegerHigh));

		// ushort -> uchar
		uint8x8_t dstInteger = vmovn_u16(dstIntegerShort);

		// store
		vst1_u8(dst+x, dstInteger);
	}
#elif defined(__x86_64__) || defined(_M_X64) || defined(_M_IX86)
	__m128i v_zero = _mm_setzero_si128();
	for (int x = 0;x <= cSize - 8;x += 8)
	{
		// _mm_loadl_epi64
		// load lower 64bit and 
		// fill higher 64bit as 0
		__m128i srcInteger   = _mm_loadl_epi64((__m128i const *)(src + x));
		__m128i gainHalfLow  = _mm_loadl_epi64((__m128i const *)(gain + x    ));
		__m128i gainHalfHigh = _mm_loadl_epi64((__m128i const *)(gain + x + 4));

		// uchar -> ushort
		__m128i srcIntegerShort = _mm_unpacklo_epi8(srcInteger, v_zero);

		// ushort -> uint
		__m128i srcIntegerLow  = _mm_unpacklo_epi16(srcIntegerShort, v_zero);
		__m128i srcIntegerHigh = _mm_unpackhi_epi16(srcIntegerShort, v_zero);

		// uint -> float
		__m128 gainFloatLow  = _mm_cvtph_ps(gainHalfLow );
		__m128 gainFloatHigh = _mm_cvtph_ps(gainHalfHigh);

		// half -> float
		__m128 srcFloatLow  = _mm_cvtepi32_ps(srcIntegerLow );
		__m128 srcFloatHigh = _mm_cvtepi32_ps(srcIntegerHigh);

		// float * float (per elment)
		__m128 dstFloatLow  = _mm_mul_ps(srcFloatLow , gainFloatLow );
		__m128 dstFloatHigh = _mm_mul_ps(srcFloatHigh, gainFloatHigh);

		// float -> uint
		__m128i dstIntegerLow  = _mm_cvtps_epi32(dstFloatLow );
		__m128i dstIntegerHigh = _mm_cvtps_epi32(dstFloatHigh);

		// uint -> ushort
		__m128i dstIntegerShort = _mm_packs_epi32(dstIntegerLow, dstIntegerHigh);

		// ushort -> uchar
		__m128i dstInteger = _mm_packus_epi16(dstIntegerShort, v_zero);

		// store
		_mm_storel_epi64((__m128i *)(dst + x), dstInteger);

	}
#endif
}
