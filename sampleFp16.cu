
// clamp x to range [a, b]
__device__ unsigned char clamp(float x, float a, float b)
{
    return (unsigned char)(max(a, min(b, x)));
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
cudaProcessHalf(unsigned char *g_odata, short *g_indata, unsigned char* imageData, int imgw)
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

	float b = imageData[(y*imgw+x)*3  ];
	float g = imageData[(y*imgw+x)*3+1];
	float r = imageData[(y*imgw+x)*3+2];

	g_odata[(y*imgw+x)*3  ] = clamp(b * gain, 0.0f, 255.0f);
	g_odata[(y*imgw+x)*3+1] = clamp(g * gain, 0.0f, 255.0f);
	g_odata[(y*imgw+x)*3+2] = clamp(r * gain, 0.0f, 255.0f);
}

extern "C" void
launchCudaProcessHalf(dim3 grid, dim3 block, int sbytes,
						short *gain,
						unsigned char *imageInput,
						unsigned char *imageOutput,
						int imgw)
{
    cudaProcessHalf<<< grid, block, sbytes >>>(imageOutput, gain, imageInput, imgw);

}

__global__ void
cudaProcessFloat(unsigned char *g_odata, float *g_indata, unsigned char* imageData, int imgw)
{
	int tx = threadIdx.x;
	int ty = threadIdx.y;
	int bw = blockDim.x;
	int bh = blockDim.y;
	int x = blockIdx.x*bw + tx;
	int y = blockIdx.y*bh + ty;

	float gain = g_indata[y*imgw+x];

	float b = imageData[(y*imgw+x)*3  ];
	float g = imageData[(y*imgw+x)*3+1];
	float r = imageData[(y*imgw+x)*3+2];

	g_odata[(y*imgw+x)*3  ] = clamp(b * gain, 0.0f, 255.0f);
	g_odata[(y*imgw+x)*3+1] = clamp(g * gain, 0.0f, 255.0f);
	g_odata[(y*imgw+x)*3+2] = clamp(r * gain, 0.0f, 255.0f);
}

extern "C" void
launchCudaProcessFloat(dim3 grid, dim3 block, int sbytes,
						float *gain,
						unsigned char *imageInput,
						unsigned char *imageOutput,
						int imgw)
{
    cudaProcessFloat<<< grid, block, sbytes >>>(imageOutput, gain, imageInput, imgw);

}
