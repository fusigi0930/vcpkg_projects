#include <iostream>
#include <argparse/argparse.hpp>
#include "cv_utils.h"

int main(int argc, char* argv[]) {
	SInfo info;

	argparse::ArgumentParser args("options");
	args.add_argument("-d", "--device")
		.required()
		.help("specific the uvc camera device")
		.scan<'i', int>();

	args.add_argument("-f", "--format")
		.help("set the uvc format")
		.append()
		.required()
		.store_into(info.vtFmt);

	args.add_argument("-r", "--resolution")
		.help("set the uvc resolution")
		.append()
		.required()
		.store_into(info.vtRes);

	args.add_argument("-F", "--function")
		.default_value(std::string(FUNC_FPS))
		.help("select the functions");

	args.add_argument("-c", "--count")
		.default_value(int(1))
		.help("set the loop count")
		.scan<'i', int>();

	args.add_argument("-C", "--close")
		.default_value(int(0))
		.help("close the camera every time")
		.scan<'i', int>();

	args.add_argument("-t", "--type")
		.default_value(std::string("list"))
		.help("set run type for all of the parameters");

	try {
		args.parse_args(argc, argv);
	}
	catch (const std::exception& err) {
		std::cerr << err.what() << std::endl;
		std::cerr << args;
		return 1;
	}


	int device = args.get<int>("--device");
	info.frameCount = 900;
	info.type = args.get<std::string>("--type");
	int count = args.get<int>("--count");
	int close = args.get<int>("--close");
	

	std::map<std::string, cv_funcs> mapFuncs;
	mapFuncs[FUNC_FPS] = cv_fps;

	if (mapFuncs.find(args.get<std::string>("--function")) == mapFuncs.end()) {
		std::cerr << "unsupport functions" << std::endl;
		return INVALID_PARAM;
	}

	if (close == 1) {
		for (int i=0; i<count; i++) {
			cv::VideoCapture cam;
			cam.open(device, cv::CAP_ANY);
			cv_fps(cam, info);
			cam.release();
		}
	}
	else {
		cv::VideoCapture cam;
		cam.open(device, cv::CAP_ANY);
		for (int i=0; i<count; i++) {
			mapFuncs[args.get<std::string>("--function")](cam, info);
		}
		cam.release();
	}
	return 0;
}