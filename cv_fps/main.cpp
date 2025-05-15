#include <iostream>
#include <sstream>

#include <argparse/argparse.hpp>
#include "cv_utils.h"
#include "uvc_utils.h"

int main(int argc, char* argv[]) {
	SInfo info;

	argparse::ArgumentParser args("options");
	args.add_argument("-d", "--device")
		.default_value(int(0))
		.help("specific the uvc camera device")
		.scan<'i', int>();

	args.add_argument("-f", "--format")
		.help("set the uvc format")
		.append()
		.store_into(info.vtFmt);

	args.add_argument("-r", "--resolution")
		.help("set the uvc resolution")
		.append()
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

	args.add_argument("--support_list")
		.implicit_value(true)
		.default_value(false)
		.help("list uvc device support format and res");

	args.add_argument("-p", "--pid")
		.default_value(std::string("0"))
		.help("give pid to list");

	args.add_argument("-v", "--vid")
		.default_value(std::string("0"))
		.help("give vid to list");

	args.add_argument("--full_fps")
		.implicit_value(true)
		.default_value(false)
		.help("run full fps");

	try {
		args.parse_args(argc, argv);
	}
	catch (const std::exception& err) {
		std::cerr << "what!!!!!!" << std::endl;
		std::cerr << err.what() << std::endl;
		std::cerr << args;
		return 1;
	}

	bool support_list = args.get<bool>("--support_list");
	bool bfull_fps = args.get<bool>("--full_fps");
	int device = args.get<int>("--device");
	info.frameCount = 900;
	info.type = args.get<std::string>("--type");
	int count = args.get<int>("--count");
	int close = args.get<int>("--close");

	std::stringstream s;
	s << std::hex << args.get<std::string>("--vid");
	int vid, pid;
	s >> vid;
	s.clear(); s << std::hex << args.get<std::string>("--pid");
	s >> pid;

	std::map<std::string, cv_funcs> mapFuncs;
	mapFuncs[FUNC_FPS] = cv_fps;

	if (support_list) {
		std::cout << "find uvc device: " << std::hex << vid << ":" << std::hex << pid << std::endl;
		suvc_list(reinterpret_cast<void*>(vid), reinterpret_cast<void*>(pid));
		return 0;
	}

	if (bfull_fps) {
		std::cout << "find uvc device: " << std::hex << vid << ":" << std::hex << pid << std::endl;
	    SUvc uvc = {0}; 
		if (SUVC_SUCCESS != suvc_init(&uvc, vid, pid)) {
			std::cerr << "init failed" << std::endl;
			return -1;
		}
		std::vector<SSupport> vtSupport;
		suvc_get_support(&uvc, vtSupport);
		suvc_close(&uvc);
		cv::VideoCapture cam;
		cam.open(device, cv::CAP_ANY);
		full_fps(&cam, &vtSupport);
		return 0;
	}

	if (mapFuncs.find(args.get<std::string>("--function")) == mapFuncs.end()) {
		std::cerr << "unsupport functions" << std::endl;
		return INVALID_PARAM;
	}

	if (close == 1) {
		for (int i=0; i<count; i++) {
			cv::VideoCapture cam;
			cam.open(device, cv::CAP_ANY);
			cv_fps(&cam, &info);
			cam.release();
		}
	}
	else {
		cv::VideoCapture cam;
		cam.open(device, cv::CAP_ANY);
		for (int i=0; i<count; i++) {
			mapFuncs[args.get<std::string>("--function")](&cam, &info);
		}
		cam.release();
	}
	return 0;
}