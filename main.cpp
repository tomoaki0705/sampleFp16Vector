#include <intrin.h>
#include <iostream>
#include <iomanip>

const int ALIGN=32; // alignment step for SSE
const int cWidth  = 256;
const int cHeight = cWidth;
const int cSize   = cHeight * cWidth;

__declspec(noinline) void float2half(float* floats, short* halfs) {
    __m256 float_vector = _mm256_load_ps(floats);
	__m128i half_vector = _mm256_cvtps_ph(float_vector, 0);
	*(__m128i*)halfs = half_vector;
}

int main()
{
	unsigned char* image =reinterpret_cast<unsigned char*>(_aligned_malloc(cSize,ALIGN));
	short* gain  =reinterpret_cast<short*>(_aligned_malloc(cSize*2,ALIGN));
	float* gainOriginal = reinterpret_cast<float*>(_aligned_malloc(cSize*4,ALIGN));
	for (unsigned int i = 0;i < cSize;i++)
	{
		gainOriginal[i] = 1.0f;
	}
	for(unsigned int i = 0;i < cSize/8;i++)
	{
		float2half(&gainOriginal[i*8], &gain[i*8]);
	}
	for (unsigned int i = 0; i < cSize; i++)
	{
		if ((i & 7) == 0)
		{
			std::cout << std::dec << std::setfill(' ') << std::setw(4) << i;
		}
		std::cout  << " 0x" << std::hex << gain[i];
		if ((i & 7) == 7)
		{
			std::cout << std::endl;
		}
	}
	return 0;
}
