
// clamp x to range [a, b]
__device__ float clamp(float x, float a, float b)
{
    return max(a, min(b, x));
}

__device__ int clamp(int x, int a, int b)
{
    return max(a, min(b, x));
}

// convert floating point rgb color to 8-bit integer
__device__ int rgbToInt(float r, float g, float b)
{
    r = clamp(r, 0.0f, 255.0f);
    g = clamp(g, 0.0f, 255.0f);
    b = clamp(b, 0.0f, 255.0f);
    return (int(b)<<16) | (int(g)<<8) | int(r);
}

__global__ void
cudaProcessHalf(unsigned int *g_odata, short *g_indata, unsigned int* imageData, int imgw)
{
	int tx = threadIdx.x;
	int ty = threadIdx.y;
	int bw = blockDim.x;
	int bh = blockDim.y;
	int x = blockIdx.x*bw + tx;
	int y = blockIdx.y*bh + ty;

	unsigned short a = g_indata[y*imgw+x];
	float gain;
	gain = __half2float(a);

	unsigned int p = imageData[y*imgw+x];

	float b = (float)((p >> 16) & 0xff);
	float g = (float)((p >>  8) & 0xff);
	float r = (float)((p      ) & 0xff);

	uchar4 c4;
	c4.x = (unsigned char)(b * gain);
	c4.y = (unsigned char)(g * gain);
	c4.z = (unsigned char)(r * gain);
	g_odata[y*imgw+x] = rgbToInt(c4.z, c4.y, c4.x);
}

extern "C" void
launchCudaProcessHalf(dim3 grid, dim3 block, int sbytes,
						short *gain,
						unsigned int *imageInput,
						unsigned int *imageOutput,
						int imgw)
{
    cudaProcessHalf<<< grid, block, sbytes >>>(imageOutput, gain, imageInput, imgw);

}

__global__ void
cudaProcessFloat(unsigned int *g_odata, float *g_indata, unsigned int* imageData, int imgw)
{
	int tx = threadIdx.x;
	int ty = threadIdx.y;
	int bw = blockDim.x;
	int bh = blockDim.y;
	int x = blockIdx.x*bw + tx;
	int y = blockIdx.y*bh + ty;

	float gain = g_indata[y*imgw+x];

	unsigned int p = imageData[y*imgw+x];

	float b = (float)((p >> 16) & 0xff);
	float g = (float)((p >>  8) & 0xff);
	float r = (float)((p      ) & 0xff);

	uchar4 c4;
	c4.x = (unsigned char)(b * gain);
	c4.y = (unsigned char)(g * gain);
	c4.z = (unsigned char)(r * gain);
	g_odata[y*imgw+x] = rgbToInt(c4.z, c4.y, c4.x);
}

extern "C" void
launchCudaProcessFloat(dim3 grid, dim3 block, int sbytes,
						float *gain,
						unsigned int *imageInput,
						unsigned int *imageOutput,
						int imgw)
{
    cudaProcessFloat<<< grid, block, sbytes >>>(imageOutput, gain, imageInput, imgw);

}
