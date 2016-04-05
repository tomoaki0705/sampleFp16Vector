#include <iostream>
#include <iomanip>
#include "platformMalloc.h"
#include "floatMul.h"
#include "featureSupport.h"
#include "simdUtils.h"

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
	float4 srcFloat1 = set_float4(1.1f, 1.49f, 1.50f, 1.99f);
	float4 srcFloat2 = set_float4(1.0000001f, 1.4999999f, 1.5000000f, 1.9999999f);
	uint4 dstInt1 = convert_float4_uint4(srcFloat1);
	uint4 dstInt2 = convert_float4_uint4(srcFloat2);

	std::cout << get_float(srcFloat1, 0) << '\t' << get_float(srcFloat1, 1) << '\t' << get_float(srcFloat1, 2) << '\t' << get_float(srcFloat1, 3) << std::endl;
	std::cout << get_uint(dstInt1, 0)    << '\t' << get_uint(dstInt1, 1)    << '\t' << get_uint(dstInt1, 2)    << '\t' << get_uint(dstInt1, 3)    << std::endl;
	std::cout << get_float(srcFloat2, 0) << '\t' << get_float(srcFloat2, 1) << '\t' << get_float(srcFloat2, 2) << '\t' << get_float(srcFloat2, 3) << std::endl;
	std::cout << get_uint(dstInt2, 0)    << '\t' << get_uint(dstInt2, 1)    << '\t' << get_uint(dstInt2, 2)    << '\t' << get_uint(dstInt2, 3)    << std::endl;

	const unsigned int cParallel = 8;
	uchar8 step     = load_uchar8_repeat((unsigned char)cParallel);
	uchar8 original = set_uchar8(0, 1, 2, 3, 4, 5, 6, 7);
	char before[cParallel], after[cParallel];
	bool passedFlag = true;
	for (int src = 1;src < 256;src++)
	{
		float4 srcVectorDiv = load_float4_copy(1.0f/(float)src);
		float4 srcVectorMul = load_float4_copy(     (float)src);
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
