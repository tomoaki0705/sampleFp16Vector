#include <iostream>
#include <iomanip>
#include <malloc.h>
#ifdef _MSC_VER
#include <cstdlib>
#define alignedMalloc(size,align)	_aligned_malloc(size,align)
#elif defined (__GNUC__)
#define alignedMalloc(size,align)	memalign(align,size)
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
	const unsigned char cParallel = 8;
	unsigned char _step[]     = {cParallel, cParallel, cParallel, cParallel, cParallel, cParallel, cParallel, cParallel, };
	unsigned char _original[] = {0, 1, 2, 3, 4, 5, 6, 7, };
	uchar8 step     = load_uchar8(_step);
	uchar8 original = load_uchar8(_original);
	char before[cParallel], after[cParallel];
	float* _srcVectorDiv =reinterpret_cast<float*>(alignedMalloc(4,ALIGN));
	float* _srcVectorMul =reinterpret_cast<float*>(alignedMalloc(4,ALIGN));
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
