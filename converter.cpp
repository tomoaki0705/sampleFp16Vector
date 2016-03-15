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
const char defaultResultFileHalf[]  = "dumpHalf.png";
const char defaultResultFileFloat[] = "dumpFloat.png";
const char defaultResultFileGray[]  = "dumpGray.png";

void convertUchar2Fp16(const cv::Mat src, cv::Mat &dstFloat, cv::Mat &dstHalf)
{
	dstFloat = cv::Mat(src.rows, src.cols, CV_32F);
	dstHalf  = cv::Mat(src.rows, src.cols, CV_16S);
	float* floatPointer = (float*)dstFloat.data;
	short* halfPointer  = (short*)dstHalf.data;
	for (int y = 0; y < src.rows; y++)
	{
	for (int x = 0; x < src.cols; x++)
	{
		unsigned int index = y*src.cols+x;
		floatPointer[index] = (float)(src.data[index]/cHigh);
	}
	}
	float2half(floatPointer, halfPointer, src.cols*src.rows);
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

bool forceFormatWrite(const cv::String &filename, cv::Mat &image, unsigned int width, unsigned int format)
{
	cv::Mat stub = cv::Mat(image.rows, width, format, image.data);
	return cv::imwrite(filename, stub);
}


int main(int argc, char** argv)
{
	if (checkFeatureSupport() == false)
	{
		return 1;
	}

	cv::Mat image;

	prepareSourceImage(argc, argv, image);

	cv::Mat floatImage;
	cv::Mat halfImage;
	convertUchar2Fp16(image, floatImage, halfImage);

	forceFormatWrite(defaultResultFileGray,  image,      image.cols,       CV_8UC1);
	forceFormatWrite(defaultResultFileFloat, floatImage, floatImage.cols,  CV_8UC4);
	forceFormatWrite(defaultResultFileHalf,  halfImage,  halfImage.cols*2, CV_8UC1);

	return 0;
}
