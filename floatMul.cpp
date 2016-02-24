
#if defined(__arm__) || defined(_M_ARM)
#include <arm_neon.h>
#elif defined(__x86_64__) || defined(_M_X64) || defined(_M_IX86)
#include <immintrin.h>
#endif
#include "floatMul.h"

// requires AVX
void float2half(float* src, short* dst, int length) {
	const unsigned int cParallel = 8;
	for (int i = 0; i <= length-cParallel; i+=cParallel)
	{
		__m256 float_vector = _mm256_load_ps(src + i);
		__m128i half_vector = _mm256_cvtps_ph(float_vector, 0);
		*(__m128i*)(dst + i) = half_vector;
	}

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
}
