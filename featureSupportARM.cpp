
#if defined(__arm__) || defined(_M_ARM) || defined(__aarch64__)
#include "featureSupport.h"
#include <unistd.h>
#include <sys/auxv.h>
#include <fcntl.h>
#include <iostream>

unsigned int getAuxv()
{
	Elf32_auxv_t auxv;
	int cpufile = open("/proc/self/auxv", O_RDONLY);
	if (cpufile >= 0)
	{
		const size_t size_auxv_t = sizeof(auxv);
		while ((size_t)read(cpufile, &auxv, size_auxv_t) == size_auxv_t)
		{
			if (auxv.a_type == AT_HWCAP)
			{
				return auxv.a_un.a_val;
			}
		}
	}
}

bool hasNeonSupport()
{
#if defined(__arm__) || defined(_M_ARM)
	return getAuxv() & HWCAP_ARM_NEON;
#elif defined(__aarch64__)
	return true;
#endif
}

bool hasF16cSupport()
{
#if defined(__arm__) || defined(_M_ARM)
	return getAuxv() & HWCAP_ARM_HALF;
#elif defined(__aarch64__)
	return true;
#endif
}

bool checkFeatureSupport()
{
	bool hasEnoughSupport = true;
	if (hasF16cSupport() == false)
	{
		std::cerr << "Processor has no fp16 support" << std::endl;
		hasEnoughSupport = false;
	}
	if (hasNeonSupport() == false)
	{
		std::cerr << "Processor has no NEON support" << std::endl;
		hasEnoughSupport = false;
	}
	return hasEnoughSupport;
}

#endif
