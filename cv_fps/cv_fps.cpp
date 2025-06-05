#include "cv_utils.h"
#include <argparse/argparse.hpp>
#include "uvc_utils.h"

#include <algorithm>
#include <random>
#include <sstream>

static void calc_fps(cv::VideoCapture &cam, SInfo &info) {
	cv::Mat f;
	uint64_t start_time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	for (int i = 0; i < info.frameCount; i++) {
		cam.read(f);
		std::cout << "\rframe: " << i << " " << f.size().width << "x" << f.size().height;
	}
	std::cout << std::endl;

	uint64_t end_time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

	long double fps = static_cast<long double>(1000.0) / ((static_cast<long double>(end_time) - static_cast<long double>(start_time)) / 1000000.0);
	std::cout << "fps: " << std::setprecision(10) << fps << std::endl;
}

#define FRAME_COUNT 450
static void calc_fps(cv::VideoCapture &cam, int device = 0, double rate = 0.0) {
	cv::Mat f;
	std::cout << std::dec;
    int max_count = FRAME_COUNT;
    int ignore_frames_count = 300;
    if (0.0 != rate) {
        cam.set(cv::CAP_PROP_FPS, rate);
        max_count = static_cast<int>(rate) * 15;
        ignore_frames_count = static_cast<int>(rate) * 10;
    }
    // drop frames
    for (int i = 0; i < ignore_frames_count; i++) {
        cam.read(f);
    }

	uint64_t start_time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	for (int i = 0; i < max_count; i++) {
		cam.read(f);
	}
	uint64_t end_time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

	long double fps = static_cast<long double>(max_count) / (static_cast<long double>(end_time- start_time) / 1000000.0);
	std::cout << "fps: " << std::setprecision(10) << fps;
	if (rate > 0.0 && rate-fps > 1.0) {
		std::cout << " ---- the real fps is less than the expectation over 1.0 ";
	}
	std::cout << std::endl;
}

int cv_fps(void *pCam, void *pInfo) {
	cv::VideoCapture *cam = reinterpret_cast<cv::VideoCapture*>(pCam);
	SInfo *info = reinterpret_cast<SInfo*>(pInfo);
	if (nullptr == cam || nullptr == info) {
		return DEVICE_ERROR;
	}

	if (!cam->isOpened()) {
		std::cerr << "open camera failed" << std::endl;
		return DEVICE_ERROR;		
	}

	if (info->type == "list") {
		for (std::vector<std::string>::iterator p=info->vtFmt.begin(); p!=info->vtFmt.end(); p++) {
			int ret = setCVFormat(*cam, *p);
			if (ret != SUCCESS) {
				return ret;
			}

			for (std::vector<std::string>::iterator q=info->vtRes.begin(); q!=info->vtRes.end(); q++) {
				ret = setCVRes(*cam, *q);
				if (ret != SUCCESS) {
					return ret;
				}

				calc_fps(*cam, *info);
			}			
		}
	}
	else if (info->type == "random") {
		std::vector<std::string> outFmt, outRes;
		std::sample(info->vtFmt.begin(), info->vtFmt.end(), std::back_inserter(outFmt), 1, std::mt19937{std::random_device{}()});
		std::sample(info->vtRes.begin(), info->vtRes.end(), std::back_inserter(outRes), 1, std::mt19937{std::random_device{}()});
		
		int ret = setCVFormat(*cam, *outFmt.begin());
		if (ret != SUCCESS) {
			return ret;
		}
		ret = setCVRes(*cam, *outRes.begin());
		if (ret != SUCCESS) {
			return ret;
		}

		calc_fps(*cam, *info);
	}

	return SUCCESS;
}

int full_fps(void *pCam, void *pSupport) {
	cv::VideoCapture *cam = reinterpret_cast<cv::VideoCapture*>(pCam);
	SSupport *support = reinterpret_cast<SSupport *>(pSupport);
	if (nullptr == cam || nullptr == support) {
		return DEVICE_ERROR;
	}

	if (!cam->isOpened()) {
		std::cerr << "open camera failed" << std::endl;
		return DEVICE_ERROR;		
	}

	for (mapSupport::iterator p = support->mapSup.begin(); p != support->mapSup.end(); p++) {
		int ret = setCVFormat(*cam, p->first);
		if (ret != SUCCESS) {
			std::cerr << __FUNCTION__ << ":set cv format failed" << std::endl;
			continue;
		}

		for (std::vector<SRes>::iterator q = p->second.begin(); q != p->second.end(); q++) {
			ret = setCVRes(*cam, q->w, q->h);
			if (ret != SUCCESS) {
				std::cerr << __FUNCTION__ << ":set cv res failed" << std::endl;
				continue;
			}
			std::cout << std::dec << "calc " << p->first << " " << q->w << " x " << q->h << ", rate = " << q->rate << " ... ";
			calc_fps(*cam, support->device, q->rate);
		}			
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