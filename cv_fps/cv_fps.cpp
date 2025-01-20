#include "cv_utils.h"
#include <argparse/argparse.hpp>

#include <algorithm>
#include <random>

static void calc_fps(cv::VideoCapture &cam, SInfo &info) {
	cv::Mat f;
	uint64_t start_time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	for (int i = 0; i < info.frameCount; i++) {
		cam.read(f);
		std::cout << "\rframe: " << i << " " << f.size().width << "x" << f.size().height;
	}
	std::cout << std::endl;

	uint64_t end_time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

	double fps = static_cast<double>(1000.0) / ((static_cast<double>(end_time) - static_cast<double>(start_time)) / 1000000.0);
	std::cout << "fps: " << fps << std::endl;
}

int cv_fps(cv::VideoCapture &cam, SInfo &info) {
	if (!cam.isOpened()) {
		std::cerr << "open camera failed" << std::endl;
		return DEVICE_ERROR;		
	}

	if (info.type == "list") {
		for (std::vector<std::string>::iterator p=info.vtFmt.begin(); p!=info.vtFmt.end(); p++) {
			int ret = setCVFormat(cam, *p);
			if (ret != SUCCESS) {
				return ret;
			}

			for (std::vector<std::string>::iterator q=info.vtRes.begin(); q!=info.vtRes.end(); q++) {
				ret = setCVRes(cam, *q);
				if (ret != SUCCESS) {
					return ret;
				}

				calc_fps(cam, info);
			}			
		}
	}
	else if (info.type == "random") {
		std::vector<std::string> outFmt, outRes;
		std::sample(info.vtFmt.begin(), info.vtFmt.end(), std::back_inserter(outFmt), 1, std::mt19937{std::random_device{}()});
		std::sample(info.vtRes.begin(), info.vtRes.end(), std::back_inserter(outRes), 1, std::mt19937{std::random_device{}()});
		
		int ret = setCVFormat(cam, *outFmt.begin());
		if (ret != SUCCESS) {
			return ret;
		}
		ret = setCVRes(cam, *outRes.begin());
		if (ret != SUCCESS) {
			return ret;
		}

		calc_fps(cam, info);
	}

	return SUCCESS;
}

# if 0
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
#endif