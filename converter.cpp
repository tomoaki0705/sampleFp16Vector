#include <opencv2/imgcodecs/imgcodecs.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "floatMul.h"
#include "featureSupport.h"
#include <string>
#include <iostream>

const unsigned int defaultWidth  = 640;
const unsigned int defaultHeight = 480;
const double cHigh = 255.0f;
const char defaultResultFile[] = "dump.png";

void convertUchar2Fp16(cv::Mat& image)
{
	cv::Mat srcFloat  = cv::Mat(image.rows, image.cols, CV_32F);
	float* floatPointer = (float*)srcFloat.data;
	for (int y = 0; y < image.rows; y++)
	{
	for (int x = 0; x < image.cols; x++)
	{
		floatPointer[y*image.cols+x] = (float)(image.data[y*image.cols+x]/cHigh);
	}
	}
	image = cv::Mat(image.rows, image.cols, CV_16S);
	short* halfPointer  = (short*)image.data;
	float2half(floatPointer, halfPointer, image.cols*image.rows);
	return ;

}


void fillWithShadingGain(cv::Mat &image)
{
	cv::Point2d center(image.cols/2, image.rows/2);
	double maxDistance = sqrt(center.x*center.x+center.y*center.y);
	for (int y = 0; y < image.rows; y++)
	{
	for (int x = 0; x < image.cols; x++)
	{
		double dx = center.x - x;
		double dy = center.y - y;
		double distance = sqrt(dx * dx + dy * dy);
		double ratio = 1.0f - (distance / maxDistance);
		image.data[y*image.cols+x] = (unsigned char)(cHigh * ratio);
	}
	}
}

void prepareSourceImage(int argc, char**argv, cv::Mat& image)
{
	std::string filename;
	if (argc >= 2)
	{
		filename = std::string(argv[1]);
	}
	else
	{
		filename = "";
	}
	if (filename != std::string(""))
	{
		image = cv::imread(filename, cv::IMREAD_GRAYSCALE);
	}
	else
	{
		image = cv::Mat(defaultHeight, defaultWidth, CV_8UC1);
		fillWithShadingGain(image);
	}
}

bool checkFeatureSupport()
{
	bool hasEnoughSupport = true;
	if (hasF16cSupport() == false)
	{
		std::cerr << "Processor has no fp16 support" << std::endl;
		hasEnoughSupport = false;
	}
#if defined(__x86_64__) || defined(_M_X64) || defined(_M_IX86) || defined(i386)
	if (hasSse41Support() == false)
	{
		std::cerr << "Processor has no SSE4.1 support" << std::endl;
		hasEnoughSupport = false;
	}
#endif
#if defined(__arm__) || defined(_M_ARM)
	if (hasNeonSupport() == false)
	{
		std::cerr << "Processor has no NEON support" << std::endl;
		hasEnoughSupport = false;
	}
#endif
	return hasEnoughSupport;
}

int main(int argc, char** argv)
{
	if (checkFeatureSupport() == false)
	{
		return 1;
	}

	cv::Mat image;

	prepareSourceImage(argc, argv, image);

	convertUchar2Fp16(image);

	cv::Mat stub = cv::Mat(image.rows, image.cols*2, CV_8UC1, image.data);

	cv::imwrite(defaultResultFile, stub);

	return 0;
}
