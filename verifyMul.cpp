#include <iostream>
#include <iomanip>
#include <malloc.h>
#ifdef _MSC_VER
#include <cstdlib>
#define alignedMalloc(size,align)	_aligned_malloc(size,align)
#define alignedFree(ptr)			_aligned_free(ptr)
#elif defined (__GNUC__)
#define alignedMalloc(size,align)	memalign(align,size)
#define alignedFree(ptr)			free(ptr)
#endif
#include "floatMul.h"
#include "featureSupport.h"
#include "simdUtils.h"

const int ALIGN   = 16; // alignment step for SSE
const int cWidth  = 256;
const int cHeight = cWidth;
const int cSize   = cHeight * cWidth;

half4 compressToHalf4(uint4 integer, float4 srcVector)
{
	float4 dstFloat4;
	dstFloat4 = convert_uint4_float4(integer);
	dstFloat4 = multiply_float4(dstFloat4, srcVector);
	return convert_float4_half4(dstFloat4);
}

uint4 uncompressToInt4(half4 halfVector, float4 srcVector)
{
	float4 dstFloat4;
	dstFloat4 = convert_half4_float4(halfVector);
	dstFloat4 = multiply_float4(dstFloat4, srcVector);
	return convert_float4_uint4(dstFloat4);
}


bool verifyMultiply()
{
	float _originalFloat[] = {1.1f,  1.49f, 1.50f, 1.99f, 1.0000001f, 1.4999999f, 1.5000000f, 1.9999999f, };
	float* originalFloat = reinterpret_cast<float*>(alignedMalloc(8*sizeof(float),ALIGN));
	unsigned int* converted = reinterpret_cast<unsigned int*>(alignedMalloc(8*sizeof(float),ALIGN));
	for (int i = 0;i < 8;i++)
	{
		originalFloat[i] = _originalFloat[i];
	}

	float4 srcFloat1 = load_float4((const float*)originalFloat);
	float4 srcFloat2 = load_float4((const float*)&originalFloat[4]);
	uint4 dstInt1 = convert_float4_uint4(srcFloat1);
	uint4 dstInt2 = convert_float4_uint4(srcFloat2);
	store_uint4(&converted[0], dstInt1);
	store_uint4(&converted[4], dstInt2);

	std::cout << originalFloat[0] << '\t' << originalFloat[1] << '\t' << originalFloat[2] << '\t' << originalFloat[3] << std::endl;
	std::cout << converted[0]     << '\t' << converted[1]     << '\t' << converted[2]     << '\t' << converted[3]     << std::endl;
	std::cout << originalFloat[4] << '\t' << originalFloat[5] << '\t' << originalFloat[6] << '\t' << originalFloat[7] << std::endl;
	std::cout << converted[4]     << '\t' << converted[5]     << '\t' << converted[6]     << '\t' << converted[7]     << std::endl;

	const unsigned char cParallel = 8;
	unsigned char _original[] = {0, 1, 2, 3, 4, 5, 6, 7, };
	uchar8 step     = load_uchar8_repeat(cParallel);
	uchar8 original = load_uchar8(_original);
	char before[cParallel], after[cParallel];
	float* _srcVectorDiv = reinterpret_cast<float*>(alignedMalloc(4*sizeof(float),ALIGN));
	float* _srcVectorMul = reinterpret_cast<float*>(alignedMalloc(4*sizeof(float),ALIGN));
	bool passedFlag = true;
	for (int src = 1;src < 256;src++)
	{
		for (int i = 0;i < 4;i++)
		{
			_srcVectorDiv[i] = 1.0f/(float)src;
			_srcVectorMul[i] = (float)src;
		}
		float4 srcVectorDiv = load_float4(_srcVectorDiv);
		float4 srcVectorMul = load_float4(_srcVectorMul);
		uchar8 dst = original;
		for (int i = 0;i <= 256 - cParallel;i += cParallel)
		{
			// compress the gain
			ushort8 dstShort8     = convert_uchar8_ushort8(dst);
			uint4 dstIntegerLow4  = convert_ushort8_lo_uint4(dstShort8);
			uint4 dstIntegerHigh4 = convert_ushort8_hi_uint4(dstShort8);
			half4 compressLow4    = compressToHalf4(dstIntegerLow4,  srcVectorDiv);
			half4 compressHigh4   = compressToHalf4(dstIntegerHigh4, srcVectorDiv);

			// uncompress the gain
			dstIntegerLow4  = uncompressToInt4(compressLow4,  srcVectorMul);
			dstIntegerHigh4 = uncompressToInt4(compressHigh4, srcVectorMul);
			dstShort8       = convert_uint4_ushort8(dstIntegerLow4, dstIntegerHigh4);
			uchar8 resultChar8;
			resultChar8     = convert_ushort8_uchar8(dstShort8);

			store_uchar8((void*)before, dst);			
			store_uchar8((void*)after,  resultChar8);
			dst = dst + step;
			for (int j = 0;j < cParallel;j++)
			{
				if (before[j] != after[j])
				{
					std::cerr << "error: src:" << src << " expected:" << (unsigned int)before[j] << " actual:" << (unsigned int)after[j] << std::endl;
					passedFlag = false;
				}
			}
		}
	}
	alignedFree(_srcVectorMul);
	alignedFree(_srcVectorDiv);
	alignedFree(converted);
	alignedFree(originalFloat);
	return passedFlag;
}

int main()
{
	if (checkFeatureSupport() == false)
	{
		return 1;
	}
	verifyMultiply();
}
