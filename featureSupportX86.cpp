
#if defined(__x86_64__) || defined(_M_X64) || defined(_M_IX86) || defined(i386)
#include "featureSupport.h"
#ifdef _MSC_VER
#include <intrin.h>
#elif defined (__GNUC__)
#include <cpuid.h>
#endif

int getCpuIdMax()
{
#ifdef _MSC_VER
	int cpuFeature[4];
	__cpuid(cpuFeature, 0);
#elif defined (__GNUC__)
	unsigned int cpuFeature[4];
	__get_cpuid_max (0, cpuFeature);
#endif
	return (int)cpuFeature[0];
}

unsigned int getFeatureEcx()
{
#ifdef _MSC_VER
	int cpuFeature[4];
	__cpuidex(cpuFeature, 1, 0);
#elif defined (__GNUC__)
	unsigned int cpuFeature[4];
	__get_cpuid (1, &cpuFeature[0], &cpuFeature[1], &cpuFeature[2], &cpuFeature[3]);
#endif
	return (unsigned int)cpuFeature[2];
}

bool hasSse41Support()
{
	bool hasSupport = false;

	if (getCpuIdMax() >= 1)
	{
		hasSupport = getFeatureEcx() & (1 << 19) ? true : false;
	}
	return hasSupport;
}

bool hasF16cSupport()
{
	bool hasSupport = false;

	if (getCpuIdMax() >= 1)
	{
		hasSupport = getFeatureEcx() & (1 << 29) ? true : false;
	}
	return hasSupport;
}

#endif
