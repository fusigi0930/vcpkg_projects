#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>
#include <string>
#include <map>
#include <chrono>
#include <vector>

#define SUCCESS			0
#define INVALID_PARAM	-1
#define DEVICE_ERROR	-2

#define FUNC_FPS		"fps"

struct SInfo {
	std::vector<std::string> vtFmt;
	std::vector<std::string> vtRes;
	int frameCount;
	std::string type;
};

typedef int (*cv_funcs) (cv::VideoCapture &, SInfo &);
int cv_fps(cv::VideoCapture &cam, SInfo &info);

int setCVRes(cv::VideoCapture &cam, std::string res);
int setCVRes(cv::VideoCapture &cam, int width, int height);
int setCVFormat(cv::VideoCapture &cam, std::string fmt);