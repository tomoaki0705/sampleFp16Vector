
#if defined(__arm__) || defined(_M_ARM)
#include "featureSupport.h"
#include <unistd.h>
#include <sys/auxv.h>
#include <fcntl.h>

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
	bool hasSupport = false;

	return getAuxv() & HWCAP_ARM_NEON;
}

bool hasF16cSupport()
{
	bool hasSupport = false;

	return getAuxv() & HWCAP_ARM_HALF;
}

#endif
