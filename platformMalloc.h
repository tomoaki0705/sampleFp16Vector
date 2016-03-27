#include <malloc.h>
#ifdef _MSC_VER
#include <cstdlib>
#define alignedMalloc(size,align)	_aligned_malloc(size,align)
#define alignedFree(ptr)			_aligned_free(ptr)
#elif defined (__GNUC__)
#define alignedMalloc(size,align)	memalign(align,size)
#define alignedFree(ptr)			free(ptr)
#endif

