#include "platformMallocDefines.h"
#include <malloc.h>
#ifdef _MSC_VER
#include <cstdlib>
inline void* alignedMalloc(size_t size,size_t align) { return _aligned_malloc(size,align); }
#define alignedFree(ptr)			                 _aligned_free(ptr)
#elif defined (__GNUC__)
#include <stdlib.h>
inline void* alignedMalloc(size_t size,size_t align) { void* a; volatile int b = posix_memalign(&a,align,size); return a; }
#define alignedFree(ptr)			                 free(ptr)
#endif

