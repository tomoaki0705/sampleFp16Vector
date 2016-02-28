

bool hasSse41Support();
bool hasF16cSupport();
void float2half(float* src, short* dst, unsigned int length);
void multiply_float(unsigned char* src, float* gain, unsigned char* dst, unsigned int cSize);
void multiply(unsigned char* src, short* gain, unsigned char* dst, unsigned int cSize);
