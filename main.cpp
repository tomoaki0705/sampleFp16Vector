#include <iostream>
#include <iomanip>
#include <malloc.h>
#ifdef _MSC_VER
extern "C"
{
#include <stdlib.h>
}
#define alignedMalloc(size,align)	_aligned_malloc(size,align)
#elif defined (__GNUC__)
#define alignedMalloc(size,align)	memalign(align,size)
#endif
#include "floatMul.h"

const int ALIGN   = 32; // alignment step for SSE
const int cWidth  = 256;
const int cHeight = cWidth;
const int cSize   = cHeight * cWidth;

int main()
{
	unsigned char* image =reinterpret_cast<unsigned char*>(alignedMalloc(cSize,ALIGN));
	short* gain  =reinterpret_cast<short*>(alignedMalloc(cSize*2,ALIGN));
	float* gainOriginal = reinterpret_cast<float*>(alignedMalloc(cSize*4,ALIGN));
	unsigned char* result =reinterpret_cast<unsigned char*>(alignedMalloc(cSize,ALIGN));
	for (unsigned int i = 0;i < cSize;i++)
	{
		gainOriginal[i] = 1.0f;
		image[i] = (i & 255);
	}
	for(unsigned int i = 0;i < cSize/8;i++)
	{
		float2half(&gainOriginal[i*8], &gain[i*8]);
	}
	multiply(image, gain, result, cSize);
	for(unsigned int i = 0;i < cSize;i++)
	{
		std::cout << (int)image[i] << ' ' << (int)result[i] << ' ';
		if((i & 7) == 7)
		{
			std::cout << std::endl;
		}
	}
	return 0;
}
