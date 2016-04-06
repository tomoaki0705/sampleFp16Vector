#include <iostream>
#include <iomanip>
#include "platformMalloc.h"
#include "floatMul.h"
#include "featureSupport.h"

const int cWidth  = 256;
const int cHeight = cWidth;
const int cSize   = cHeight * cWidth;
typedef unsigned char uchar;

bool examineTest(uchar *result, uchar *result_examine, int cSize)
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
	uchar* image          = reinterpret_cast<uchar*>(alignedMalloc(cSize*sizeof(uchar),ALIGN));
	short* gain           = reinterpret_cast<short*>(alignedMalloc(cSize*sizeof(short),ALIGN));
	float* gainOriginal   = reinterpret_cast<float*>(alignedMalloc(cSize*sizeof(float),ALIGN));
	uchar* result         = reinterpret_cast<uchar*>(alignedMalloc(cSize*sizeof(uchar),ALIGN));
	uchar* result_examine = reinterpret_cast<uchar*>(alignedMalloc(cSize*sizeof(uchar),ALIGN));
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
