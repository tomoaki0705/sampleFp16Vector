#include <opencv2/imgcodecs/imgcodecs.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "floatMul.h"
#include "featureSupport.h"
#include <string>
#include <iostream>

cv::VideoCapture capture;
const char windowName[]  = "demo";
const char header[] = "precision: ";
const char* imagePath[][2] = {
	{ "defaultMaskHalf.png", "defaultMaskFloat.png"},
	{ "lenaMaskHalf.png",    "lenaMaskFloat.png"},
};

enum precision
{
	precisionHalf,
	precisionFloat,
};
enum device
{
	useCpuSimd,
	useGpu,
};

double multiplyImage(cv::Mat &image, cv::Mat gain)
{
	cv::Mat stub, b, g, r;
	std::vector<cv::Mat> arrayColor;
	arrayColor.push_back(b);
	arrayColor.push_back(g);
	arrayColor.push_back(r);
	cv::split(image, arrayColor);
	int64 begin, end;

	begin = cv::getTickCount();
	switch (gain.elemSize())
	{
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

int main(int argc, char**argv)
{
	capture.open(0);
	cv::Mat image;
	capture >> image;

	cv::namedWindow(windowName);
	cv::imshow(windowName, image);

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
		case '0':
		case '1':
			index = key - '0';
			if (statusPrecision == precisionHalf)
			{
				// precision half
				stub = cv::imread(imagePath[index][0], cv::IMREAD_UNCHANGED);
				gain = cv::Mat(stub.rows, stub.cols/2, CV_16SC1, stub.data);
			}
			else
			{
				// precision single
				stub = cv::imread(imagePath[index][1], cv::IMREAD_UNCHANGED);
				gain = cv::Mat(stub.rows, stub.cols, CV_32FC1, stub.data);
			}
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
			// CUDA
			// empty for now
		}
		std::cout << elapsedTime * 1000.0f << "[ms]   \r";
		std::cout << std::flush;

		cv::imshow(windowName, image);
		key = cv::waitKey(1);
	}

	cv::destroyAllWindows();

	return 0;
}
