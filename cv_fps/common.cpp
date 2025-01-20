#include <iostream>
#include "cv_utils.h"

struct SRes {
	int w;
	int h;
};

int setCVRes(cv::VideoCapture &cam, std::string res) {
	std::map<std::string, SRes> mapRes;
	mapRes["4k"] = { 3840, 2160 };
	mapRes["2k"] = { 2560, 1440 };
	mapRes["1080"] = { 1920, 1080 };
	mapRes["720"] = { 1280, 720 };
	mapRes["360"] = { 640, 360 };
	mapRes["120"] = { 160, 120 };
	mapRes["144"] = { 176, 144 };
	mapRes["240"] = { 320, 240 };
	mapRes["270"] = { 480, 270 };

	std::map<std::string, SRes>::iterator pFind= mapRes.find(res);
	if (pFind == mapRes.end()) {
		std::cerr << "not support resolution!" << std::endl;
		return INVALID_PARAM;
	}

	return setCVRes(cam, pFind->second.w, pFind->second.h);
}

int setCVRes(cv::VideoCapture &cam, int width, int height) {
	if (width < 0 || height < 0) {
		std::cerr << "invalid width or height value" << std::endl;
		return INVALID_PARAM;
	}

	if (!cam.isOpened()) {
		std::cerr << "open camera failed" << std::endl;
		return DEVICE_ERROR;
	}

	cam.set(cv::CAP_PROP_FRAME_WIDTH, width);
	cam.set(cv::CAP_PROP_FRAME_HEIGHT, height);

	return SUCCESS;
}

int setCVFormat(cv::VideoCapture &cam, std::string fmt) {
	if (!cam.isOpened()) {
		std::cerr << "open camera failed" << std::endl;
		return DEVICE_ERROR;
	}

	if (fmt == "mjpeg") {
		cam.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M','J','P','G'));
	}
	else if (fmt == "yuv422") {
		cam.set(cv::CAP_PROP_CONVERT_RGB, 0);
		cam.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('Y', 'U', 'Y', 'V'));
	}

	return SUCCESS;
}