#include <opencv2/imgcodecs/imgcodecs.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "floatMul.h"
#include "featureSupport.h"
#include <string>
#include <iostream>
#include <iomanip>
#ifdef HAVE_CUDA
// CUDA includes
#include <cuda_runtime.h>
#endif // HAVE_CUDA

typedef unsigned char uchar;

cv::VideoCapture capture;
const char dumpFilename[] = "dump.png";
const char windowName[]  = "demo";
const char header[] = "precision: ";
const char* imagePath[][3] = {
	{ "defaultMaskHalf.png", "defaultMaskFloat.png", "defaultMaskGray.png"},
	{ "lenaMaskHalf.png",    "lenaMaskFloat.png",    "lenaMaskGray.png"},
};
short *gainCuda = NULL;
float *gainFloatCuda = NULL;
uchar *gainByteCuda = NULL;
uchar *imageCuda = NULL;
uchar *imageResult = NULL;
const cv::Size vgaSize = cv::Size(640,480);

enum precision
{
	precisionHalf,
	precisionFloat,
	precisionByte,
};
enum device
{
	useCpuSimd,
	useGpu,
};

void computeStatistics(double time, char key)
{
	const int cHistoryMax = 16;
	static double history[cHistoryMax];
	static double square [cHistoryMax];
	static int iHistory = 0;
	switch (key)
	{
	case 'h':
	case 'H':
	case 'f':
	case 'F':
	case 'b':
	case 'B':
	case 'c':
	case 'C':
	case 'g':
	case 'G':
		for (int i = 0; i < cHistoryMax; i++)
		{
			history[i] = 0.0f;
			square[i] = 0.0f;
		}
		iHistory = 0;
		break;
	default:
		break;
	}

	history[iHistory] = time;
	square[iHistory]  = time * time;

	double sum = 0.0f;
	double squareSum = 0.0f;
	for (int i = 0; i < cHistoryMax; i++)
	{
		sum += history[i];
		squareSum += square[i];
	}
	double average = sum / (double)cHistoryMax;
	squareSum /= cHistoryMax;
	double variance = squareSum - (average * average);
	std::cout << "average: " << std::fixed << std::setprecision(3) << average * 1000.0f << "[ms] ";
	std::cout << "stddev: "  << std::fixed << std::setprecision(3) << sqrt(variance) * 1000.0f << "[ms]  \r";
	std::cout << std::flush;

	iHistory++;
	iHistory = iHistory & (cHistoryMax-1);
}

#ifdef HAVE_CUDA
extern "C" void
launchCudaProcessHalf(dim3 grid, dim3 block, int sbytes,
						short *gain,
						uchar *imageInput,
						uchar *imageOutput,
						int imgw);

extern "C" void
launchCudaProcessFloat(dim3 grid, dim3 block, int sbytes,
						float *gain,
						uchar *imageInput,
						uchar *imageOutput,
						int imgw);

extern "C" void
launchCudaProcessByte(dim3 grid, dim3 block, int sbytes,
						uchar *gain,
						uchar *imageInput,
						uchar *imageOutput,
						int imgw);

double multiplyImageCuda(cv::Mat &image, cv::Mat gain)
{
	unsigned int image_width  = image.cols;
	unsigned int image_height = image.rows;
	unsigned int imageSizeGray  = image_width * image_height;
	unsigned int imageSizeColor = imageSizeGray * 3;

	cudaMemcpy(imageCuda, image.data, imageSizeColor*sizeof(char), cudaMemcpyHostToDevice);

	// calculate grid size
    dim3 block(16, 16, 1);
    dim3 grid(image_width / block.x, image_height / block.y, 1);
	int64 begin, end;

	switch (gain.elemSize())
	{
	case 1:
		begin = cv::getTickCount();
		cudaMemcpy(gainByteCuda,  gain.data,  imageSizeGray*sizeof(char),  cudaMemcpyHostToDevice);
		end = cv::getTickCount();
		launchCudaProcessByte(grid, block, 0, gainByteCuda, imageCuda, imageResult, image_width);
		break;
	case 2:
		begin = cv::getTickCount();
		cudaMemcpy(gainCuda,  gain.data,  imageSizeGray*sizeof(short),  cudaMemcpyHostToDevice);
		end = cv::getTickCount();
		launchCudaProcessHalf(grid, block, 0, gainCuda, imageCuda, imageResult, image_width);
		break;
	case 4:
		begin = cv::getTickCount();
		cudaMemcpy(gainFloatCuda,  gain.data,  imageSizeGray*sizeof(float),  cudaMemcpyHostToDevice);
		end = cv::getTickCount();
		launchCudaProcessFloat(grid, block, 0, gainFloatCuda, imageCuda, imageResult, image_width);
		break;
	}
	cudaMemcpy(image.data, imageResult, imageSizeColor*sizeof(char), cudaMemcpyDeviceToHost);

	double tickCountElapsed = double(end - begin);
	return tickCountElapsed/(double)cv::getTickFrequency();
}
#endif // HAVE_CUDA

double multiplyImage(cv::Mat &image, cv::Mat gain)
{
	cv::Mat stub, b, g, r, stubGain;
	std::vector<cv::Mat> arrayColor;
	arrayColor.push_back(b);
	arrayColor.push_back(g);
	arrayColor.push_back(r);
	cv::split(image, arrayColor);
	int64 begin, end;

	begin = cv::getTickCount();
	switch (gain.elemSize())
	{
	case 1:
		gain.convertTo(stubGain, CV_32FC1, 1/255.0f);
		for (unsigned int i = 0; i < arrayColor.size(); i++)
		{
			arrayColor[i].convertTo(stub, CV_32FC1);
			multiply(stub, stubGain, arrayColor[i], 1.0, CV_8UC1);
		}
		break;
	case 2:
		for (unsigned int i = 0; i < arrayColor.size(); i++)
		{
			multiply(arrayColor[i].data, (short*)gain.data, arrayColor[i].data, arrayColor[i].rows*arrayColor[i].cols);
		}
		break;
	case 4:
	default:
		for (unsigned int i = 0; i < arrayColor.size(); i++)
		{
			arrayColor[i].convertTo(stub, CV_32FC1);
			multiply(stub, gain, arrayColor[i], 1.0, CV_8UC1);
		}
		break;
	}
	end = cv::getTickCount();
	cv::merge(arrayColor, image);

	double tickCountElapsed = double(end - begin);
	return tickCountElapsed/(double)cv::getTickFrequency();
}

bool isFinish(char key)
{
	bool finishKey = false;
	switch (key)
	{
	case 'q':
	case 'Q':
	case 27: // ESC key
		finishKey = true;
	default:
		break;
	}
	return finishKey;
}

#ifdef HAVE_CUDA
void initArray(cv::Mat &image)
{
	unsigned int w = image.cols;
	unsigned int h = image.rows;
	unsigned int s = w * h;
	unsigned int c = s * 3;
	cudaMalloc((short**)&gainCuda,      (s*sizeof(short)));
	cudaMalloc((float**)&gainFloatCuda, (s*sizeof(float)));
	cudaMalloc((uchar**)&gainByteCuda,  (s*sizeof(uchar)));
	cudaMalloc((uchar**)&imageCuda,     (c*sizeof(uchar)));
	cudaMalloc((uchar**)&imageResult,   (c*sizeof(uchar)));
}

void releaseArray()
{
	cudaFree((void*)gainCuda);
	cudaFree((void*)gainFloatCuda);
	cudaFree((void*)gainByteCuda);
	cudaFree((void*)imageCuda);
	cudaFree((void*)imageResult);
}

bool initCuda()
{
	int devID = 0;
	int device_count= 0;

	cudaGetDeviceCount(&device_count);

	if (device_count < 1)
	{
		return false;
	}

	cudaSetDevice(devID);

	return true;
}
#else 
void initArray(cv::Mat &image){}
void releaseArray(){}
bool initCuda(){ return false; }
#endif // HAVE_CUDA

int main(int argc, char**argv)
{
	capture.open(0);
	if (capture.isOpened() == false)
	{
		std::cerr << "no capture device found" << std::endl;
		return 1;
	}
	capture.set(CV_CAP_PROP_FRAME_WIDTH,  vgaSize.width);
	capture.set(CV_CAP_PROP_FRAME_HEIGHT, vgaSize.height);
	if (capture.get(cv::CAP_PROP_FRAME_WIDTH) != (double)vgaSize.width || capture.get(cv::CAP_PROP_FRAME_HEIGHT) != (double)vgaSize.height)
	{
		std::cerr << "current device doesn't support " << vgaSize.width << "x" << vgaSize.height << " size" << std::endl;
		return 2;
	}
	cv::Mat image;
	capture >> image;

	cv::namedWindow(windowName);
	cv::imshow(windowName, image);

	initCuda();
	initArray(image);

	char key = -1;
	enum device statusDevice = useCpuSimd;
	enum precision statusPrecision = precisionFloat;
	int index = 1;
	cv::Mat stub = cv::imread(imagePath[index][0], cv::IMREAD_UNCHANGED);
	cv::Mat gain = cv::Mat(stub.rows, stub.cols/2, CV_16SC1, stub.data);
	double elapsedTime;
	while (isFinish(key) == false)
	{
		capture >> image;

		switch (key)
		{
		case 'h':
		case 'H':
			// switch to half precision
			statusPrecision = precisionHalf;
			std::cout << std::endl << header << "half  " << std::endl;
			stub = cv::imread(imagePath[index][0], cv::IMREAD_UNCHANGED);
			gain = cv::Mat(stub.rows, stub.cols/2, CV_16SC1, stub.data);
			break;
		case 'f':
		case 'F':
			// switch to single precision
			statusPrecision = precisionFloat;
			std::cout << std::endl << header << "single" << std::endl;
			stub = cv::imread(imagePath[index][1], cv::IMREAD_UNCHANGED);
			gain = cv::Mat(stub.rows, stub.cols, CV_32FC1, stub.data);
			break;
		case 'b':
		case 'B':
			// switch to gray gain
			statusPrecision = precisionByte;
			std::cout << std::endl << header << "char" << std::endl;
			gain = cv::imread(imagePath[index][2], cv::IMREAD_GRAYSCALE);
			break;
		case '0':
		case '1':
			index = key - '0';
			switch (statusPrecision)
			{
			case precisionHalf:
				// precision half
				stub = cv::imread(imagePath[index][0], cv::IMREAD_UNCHANGED);
				gain = cv::Mat(stub.rows, stub.cols/2, CV_16SC1, stub.data);
				break;
			case precisionFloat:
				// precision single
				stub = cv::imread(imagePath[index][1], cv::IMREAD_UNCHANGED);
				gain = cv::Mat(stub.rows, stub.cols, CV_32FC1, stub.data);
				break;
			case precisionByte:
				// precision single
				gain = cv::imread(imagePath[index][2], cv::IMREAD_GRAYSCALE);
				break;
			default:
				break;
			}
			break;
		case 'c':
		case 'C':
			std::cout << std::endl << "Using CPU SIMD           " << std::endl;
			statusDevice = useCpuSimd;
			break;
		case 'g':
		case 'G':
			std::cout << std::endl << "Using GPU                " << std::endl;
			statusDevice = useGpu;
			break;
		default:
			break;
		}

		if (statusDevice == useCpuSimd)
		{
			elapsedTime = multiplyImage(image, gain);
		}
		else
		{
#ifdef HAVE_CUDA
			// CUDA
			elapsedTime = multiplyImageCuda(image, gain);
#endif // HAVE_CUDA
		}
		computeStatistics(elapsedTime, key);

		if (key == 's' || key == 'S')
		{
			cv::imwrite(dumpFilename, image);
		}

		cv::imshow(windowName, image);
		key = cv::waitKey(1);
	}
	std::cout << std::endl;
	cv::destroyAllWindows();
	releaseArray();

	return 0;
}
