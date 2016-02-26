
#include "floatMul.h"
#include "simdUtils.h"

void float2half(float* src, short* dst, int length) {
	const unsigned int cParallel = 4;
	for (int i = 0; i <= length-cParallel; i+=cParallel)
	{
		float4 float_vector = load_float4(src + i);
		half4  half_vector  = convert_float4_half4(float_vector);
		store_half4(dst + i, half_vector);
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
	const unsigned int cParallel = 8;
	for (int x = 0;x <= cSize - cParallel;x += cParallel)
	{
		// load 64bits
		uchar8 srcInteger = load_uchar8(src+x);
		// load 32bits
		half4 gainHalfLow  = load_half4(gain + x     );
		half4 gainHalfHigh = load_half4(gain + x + 4 );

		// uchar -> ushort
		ushort8 srcIntegerShort = convert_uchar8_ushort8(srcInteger);

		// ushort -> uint
		uint4 srcIntegerLow  = convert_ushort8_lo_uint4(srcIntegerShort);
		uint4 srcIntegerHigh = convert_ushort8_hi_uint4(srcIntegerShort);

		// uint -> float
		float4 srcFloatLow  = convert_uint4_float4(srcIntegerLow );
		float4 srcFloatHigh = convert_uint4_float4(srcIntegerHigh);

		// half -> float
		float4 gainFloatLow  = convert_half4_float4(gainHalfLow );
		float4 gainFloatHigh = convert_half4_float4(gainHalfHigh);

		// float * float (per elment)
		float4 dstFloatLow  = multiply_float4(srcFloatLow , gainFloatLow );
		float4 dstFloatHigh = multiply_float4(srcFloatHigh, gainFloatHigh);

		// float -> uint
		uint4 dstIntegerLow  = convert_float4_uint4(dstFloatLow );
		uint4 dstIntegerHigh = convert_float4_uint4(dstFloatHigh);

		// uint -> ushort
		ushort8 dstIntegerShort = convert_uint4_ushort8(dstIntegerLow, dstIntegerHigh);

		// ushort -> uchar
		uchar8 dstInteger = convert_ushort8_uchar8(dstIntegerShort);

		// store
		store_uchar8(dst + x, dstInteger);
	}
}
