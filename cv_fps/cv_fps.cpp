#include <iostream>
#include <string>
#include <map>
#include <chrono>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <argparse/argparse.hpp>

struct SRes {
	int w;
	int h;
};

void cv_fps(int device, std::string fmt, std::string res) {
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
		std::cout << "not support resolution!" << std::endl;
		return;
	}

	cv::VideoCapture cam;
	int apiID = cv::CAP_ANY;
	cam.open(device, apiID);
	if (!cam.isOpened()) {
		std::cerr << "open camera failed" << std::endl;
		return;
	}

	if (fmt == "mjpeg") {
		cam.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M','J','P','G'));
	}
	else if (fmt == "yuv422") {
		cam.set(cv::CAP_PROP_CONVERT_RGB, 0);
		cam.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('Y', 'U', 'Y', 'V'));
	}

	cam.set(cv::CAP_PROP_FRAME_WIDTH, pFind->second.w);
	cam.set(cv::CAP_PROP_FRAME_HEIGHT, pFind->second.h);

	cv::Mat f;
	uint64_t start_time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	for (int i = 0; i < 1000; i++) {
		cam.read(f);
		std::cout << "\rframe: " << i << " " << f.size().width << "x" << f.size().height;
	}
	std::cout << std::endl;

	uint64_t end_time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

	double fps = static_cast<double>(1000.0) / ((static_cast<double>(end_time) - static_cast<double>(start_time)) / 1000000.0);
	std::cout << "fps: " << fps << std::endl;
}


int main(int argc, char* argv[]) {
	argparse::ArgumentParser args("options");
	args.add_argument("-d", "--device")
		.required()
		.help("specific the uvc camera device")
		.scan<'i', int>();

	args.add_argument("-f", "--format")
		.default_value(std::string("mjpeg"))
		.help("set the uvc format");

	args.add_argument("-r", "--resolution")
		.default_value(std::string("4k"))
		.help("set the uvc resolution");

	try {
		args.parse_args(argc, argv);
	}
	catch (const std::exception& err) {
		std::cerr << err.what() << std::endl;
		std::cerr << args;
		return 1;
	}

	int device = args.get<int>("--device");
	std::string fmt = args.get<std::string>("--format");
	std::string res = args.get<std::string>("--resolution");

	cv_fps(device, fmt, res);

	return 0;
}