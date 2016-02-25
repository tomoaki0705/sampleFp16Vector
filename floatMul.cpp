
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
	for (int x = 0;x <= cSize - 16;x += 16)
	{
		// load 128bits
		uint8x16_t srcInteger = vld1q_u8(src+x);
		// load 64bits
		float16x4_t gainHalfLowLow   = *(float16x4_t*)(gain + x     );
		float16x4_t gainHalfLowHigh  = *(float16x4_t*)(gain + x + 4 );
		float16x4_t gainHalfHighLow  = *(float16x4_t*)(gain + x + 8 );
		float16x4_t gainHalfHighHigh = *(float16x4_t*)(gain + x + 12);

		// uchar -> ushort
		uint16x8_t srcIntegerLow  = vmovl_u8(vget_low_u8 (srcInteger));
		uint16x8_t srcIntegerHigh = vmovl_u8(vget_high_u8(srcInteger));
		
		// ushort -> uint
		uint32x4_t srcIntegerLowLow   = vmovl_u16(vget_low_s16 (srcIntegerLow ));
		uint32x4_t srcIntegerLowHigh  = vmovl_u16(vget_high_s16(srcIntegerLow ));
		uint32x4_t srcIntegerHighLow  = vmovl_u16(vget_low_s16( srcIntegerHigh));
		uint32x4_t srcIntegerHighHigh = vmovl_u16(vget_high_s16(srcIntegerHigh));

		// half -> float
		float32x4_t gainFloatLowLow   = vcvt_f32_f16(gainHalfLowLow  );
		float32x4_t gainFloatLowHigh  = vcvt_f32_f16(gainHalfLowHigh );
		float32x4_t gainFloatHighLow  = vcvt_f32_f16(gainHalfHighLow );
		float32x4_t gainFloatHighHigh = vcvt_f32_f16(gainHalfHighHigh);

		// uint -> float
		float32x4_t srcFloatLowLow   = vcvtq_f32_u32(srcIntegerLowLow  );
		float32x4_t srcFloatLowHigh  = vcvtq_f32_u32(srcIntegerLowHigh );
		float32x4_t srcFloatHighLow  = vcvtq_f32_u32(srcIntegerHighLow );
		float32x4_t srcFloatHighHigh = vcvtq_f32_u32(srcIntegerHighHigh);

		// float * float (per elment)
		float32x4_t dstFloatLowLow   = vmulq_f32(srcFloatLowLow,   gainFloatLowLow  );
		float32x4_t dstFloatLowHigh  = vmulq_f32(srcFloatLowHigh,  gainFloatLowHigh );
		float32x4_t dstFloatHighLow  = vmulq_f32(srcFloatHighLow,  gainFloatHighLow );
		float32x4_t dstFloatHighHigh = vmulq_f32(srcFloatHighHigh, gainFloatHighHigh);

		// float -> uint
		uint32x4_t dstIntegerLowLow   = vcvtq_u32_f32(dstFloatLowLow  );
		uint32x4_t dstIntegerLowHigh  = vcvtq_u32_f32(dstFloatLowHigh );
		uint32x4_t dstIntegerHighLow  = vcvtq_u32_f32(dstFloatHighLow );
		uint32x4_t dstIntegerHighHigh = vcvtq_u32_f32(dstFloatHighHigh);

		// uint -> ushort
		uint16x8_t dstIntegerLow  = vcombine_u16(vmovn_u16(dstIntegerLowLow),  vmovn_u16(dstIntegerLowHigh));
		uint16x8_t dstIntegerHigh = vcombine_u16(vmovn_u16(dstIntegerHighLow), vmovn_u16(dstIntegerHighHigh));

		// ushort -> uchar
		uint8x16_t dstInteger = vcombine_u16(vmovn_u16(dstIntegerLow), vmovn_u16(dstIntegerHigh));
		vst1q_u8(dst+x, dstInteger);
	}
#elif defined(__x86_64__) || defined(_M_X64) || defined(_M_IX86)
	__m128i v_zero = _mm_setzero_si128();
	for (int x = 0;x <= cSize - 4;x += 4)
	{
		// _mm_loadl_epi64
		// load lower 64bit and 
		// fill higher 64bit as 0
		__m128i src_integer = _mm_loadl_epi64((__m128i const *)(src + x));
		__m128i gain_half = _mm_loadl_epi64((__m128i const *)(gain + x));

		// uchar -> ushort
		src_integer = _mm_unpacklo_epi8(src_integer, v_zero);

		// ushort -> uint
		src_integer = _mm_unpacklo_epi16(src_integer, v_zero);

		// half -> float
		__m128 src_float   = _mm_cvtepi32_ps(src_integer);

		// uint -> float
		__m128 gain_float = _mm_cvtph_ps(gain_half);

		// float * float (per elment)
		__m128 dst_float = _mm_mul_ps(src_float, gain_float);

		//__m256 _mm256_mul_ps (__m256 a, __m256 b)

		// float -> uint
		__m128i dst_integer = _mm_cvtps_epi32(dst_float);

		// uint -> ushort
		dst_integer = _mm_packs_epi32(dst_integer, v_zero);

		// ushort -> uchar
		dst_integer = _mm_packus_epi16(dst_integer, v_zero);

		// sotre
		_mm_storel_epi64((__m128i *)(dst + x), dst_integer);

	}
#endif
}
