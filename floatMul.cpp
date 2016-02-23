#include <immintrin.h>
#include <iostream>
#include <iomanip>
#include <malloc.h>
#include "floatMul.h"
#ifdef _MSC_VER
extern "C"
{
#include <stdlib.h>
}
#define alignedMalloc(size,align)	_aligned_malloc(size,align)
#elif defined (__GNUC__)
#define alignedMalloc(size,align)	memalign(align,size)
#endif

void multiply(unsigned char* src, short* gain, unsigned char* dst, unsigned int cSize)
{
	int x = 0;
	__m128i v_zero = _mm_setzero_si128();
	//__m128 v_scale = _mm_set1_ps(scale);
	for ( ; x <= cSize - 4; x += 4)
	{
		// _mm_loadl_epi64
		// load lower 64bit and 
		// fill higher 64bit as 0
		__m128i v_src1 = _mm_loadl_epi64((__m128i const *)(src + x));
		__m128i scalei = _mm_loadl_epi64((__m128i const *)(gain + x));
		//__m128i v_src2 = _mm_loadl_epi64((__m128i const *)(src2 + x));
		//_mm_load_si128
		// _mm_unpacklo_epi8
		// interleave two vectors, 8bits each
		// _mm_srai_epi16
		//	bit shift right arithmetic 16bits each
		// extend the signal bit
		// (i.e. schar -> sshort)
		//v_src1 = _mm_srai_epi16(_mm_unpacklo_epi8(v_zero, v_src1), 8);
		//v_src2 = _mm_srai_epi16(_mm_unpacklo_epi8(v_zero, v_src2), 8);

		// uchar -> ushort
		v_src1 = _mm_unpacklo_epi8(v_src1, v_zero);
		// ushort -> uint
		v_src1 = _mm_unpacklo_epi16(v_src1, v_zero);
		// half -> float
		// uint -> float
		// float * float (per elment)
		__m128 src_float   = _mm_cvtepi32_ps(v_src1);
		__m128 scale_float = _mm_cvtph_ps(scalei);
		__m128 v_dst1 = _mm_mul_ps(src_float, scale_float);
		//__m256 _mm256_mul_ps (__m256 a, __m256 b)
		// float -> uint
		// uint -> ushort
		__m128i v_dsti = _mm_packs_epi32(_mm_cvtps_epi32(v_dst1), v_zero);
		// ushort -> uint
		// sotre
		v_dsti = _mm_packus_epi16(v_dsti, v_zero);
		_mm_storel_epi64((__m128i *)(dst + x), v_dsti);

		//__m128 v_dst1 = _mm_mul_ps(_mm_cvtepi32_ps(_mm_srai_epi32(_mm_unpacklo_epi16(v_zero, v_src1), 16)),
		//							_mm_cvtepi32_ps(_mm_srai_epi32(_mm_unpacklo_epi16(v_zero, v_src2), 16)));
		//v_dst1 = _mm_mul_ps(v_dst1, v_scale);

		//__m128 v_dst2 = _mm_mul_ps(_mm_cvtepi32_ps(_mm_srai_epi32(_mm_unpackhi_epi16(v_zero, v_src1), 16)),
		//							_mm_cvtepi32_ps(_mm_srai_epi32(_mm_unpackhi_epi16(v_zero, v_src2), 16)));
		//v_dst2 = _mm_mul_ps(v_dst2, v_scale);

		//__m128i v_dsti = _mm_packs_epi32(_mm_cvtps_epi32(v_dst1), _mm_cvtps_epi32(v_dst2));
		//_mm_storel_epi64((__m128i *)(dst + x), _mm_packs_epi16(v_dsti, v_zero));
	}
}
