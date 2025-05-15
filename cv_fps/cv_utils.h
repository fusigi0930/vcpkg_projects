#ifndef __CV_UTILS_H__
#define __CV_TUILS_H__
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

typedef int (*cv_funcs) (void *, void *);
int cv_fps(void *pCam, void *pInfo);
int full_fps(void *pCam, void *pSupport);

int setCVRes(cv::VideoCapture &cam, std::string res);
int setCVRes(cv::VideoCapture &cam, int width, int height);
int setCVFormat(cv::VideoCapture &cam, std::string fmt);
#endif