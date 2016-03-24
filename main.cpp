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

const int ALIGN   = 16; // alignment step for SSE
const int cWidth  = 256;
const int cHeight = cWidth;
const int cSize   = cHeight * cWidth;

bool examineTest(unsigned char *result, unsigned char *result_examine, int cSize)
{
	bool flag = true;
	int i = 0;
	for (i = 0;i < cSize;i++)
	{
		if (result[i] != result_examine[i])
		{
			flag = false;
			break;
		}
	}
	if (flag == false)
	{
		std::cout << "Failed on " << i << " expected:" << result_examine[i] << " actual:" << result[i] << std::endl;
	}
	return flag;
}

int main()
{
	if (checkFeatureSupport() == false)
	{
		return 1;
	}
	unsigned char* image = reinterpret_cast<unsigned char*>(alignedMalloc(cSize,ALIGN));
	short* gain  = reinterpret_cast<short*>(alignedMalloc(cSize*sizeof(short),ALIGN));
	float* gainOriginal = reinterpret_cast<float*>(alignedMalloc(cSize*sizeof(float),ALIGN));
	unsigned char* result = reinterpret_cast<unsigned char*>(alignedMalloc(cSize,ALIGN));
	unsigned char* result_examine = reinterpret_cast<unsigned char*>(alignedMalloc(cSize,ALIGN));
	for (unsigned int i = 0;i < cSize;i++)
	{
		gainOriginal[i] = 1.0f;
		image[i] = (i & 255);
	}
	float2half(gainOriginal, gain, cSize);
	multiply(image, gain, result, cSize);
	multiply_float(image, gainOriginal, result_examine, cSize);

	bool examineResult = examineTest(result, result_examine, cSize);
	alignedFree(result_examine);
	alignedFree(result);
	alignedFree(gainOriginal);
	alignedFree(gain);
	alignedFree(image);
	if (examineResult == true)
	{
		std::cout << "test passed" << std::endl;
		return 0;
	}
	else
	{
		return 1;
	}
}
