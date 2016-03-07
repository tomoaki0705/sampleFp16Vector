#include <opencv2/imgcodecs/imgcodecs.hpp>
#include <opencv2/core/core.hpp>
#include <string>
#include <iostream>
const unsigned int defaultWidth  = 640;
const unsigned int defaultHeight = 480;

void fillWithShadingGain(cv::Mat &image)
{
	for (unsigned int y = 0; y < image.rows; y++)
	{
	for (unsigned int x = 0; x < image.cols; x++)
	{


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

int main(int argc, char** argv)
{
	cv::Mat image;
	return 0;
}
