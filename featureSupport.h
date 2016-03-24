

#if defined(__arm__) || defined(_M_ARM) || defined(__aarch64__)
bool hasNeonSupport();
#endif
#if defined(__x86_64__) || defined(_M_X64) || defined(_M_IX86) || defined(i386)
bool hasSse41Support();
#endif
bool hasF16cSupport();
bool checkFeatureSupport();
