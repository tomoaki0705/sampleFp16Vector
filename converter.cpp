#include <opencv2/imgcodecs/imgcodecs.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <string>
#include <iostream>
const unsigned int defaultWidth  = 640;
const unsigned int defaultHeight = 480;

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
		image.data[y*image.cols+x] = (unsigned char)(255 * ratio);
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
	prepareSourceImage(argc, argv, image);
	return 0;
}
