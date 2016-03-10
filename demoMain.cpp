#include <opencv2/imgcodecs/imgcodecs.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "floatMul.h"
#include "featureSupport.h"
#include <string>
#include <iostream>

cv::VideoCapture capture;
const char windowName[]  = "demo";

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
	while (isFinish(key) == false)
	{
		capture >> image;
		cv::imshow(windowName, image);
		key = cv::waitKey(1);
	}

	cv::destroyAllWindows();

	return 0;
}
