#ifndef __UVC_UTILS_H__
#define __UVC_TUILS_H__

#include <vector>
#include <string>
#include <cstdint>
#include <tuple>

#if !defined(_WIN32)
#  include "libuvc/libuvc.h"
struct SUvc {
    uvc_context_t *ctx;
    uvc_device_t *dev;
    uvc_device_handle_t *devh;
};
#else
#  include <dshow.h>
struct SUvc {
	ICreateDevEnum *pDevEnum;
	IEnumMoniker *pEnum;
	IPropertyBag *pPropBag;
	IMoniker *pMoniker;
	IBaseFilter *pFilter;
	IEnumPins *pEnumPins;
	IPin *pPin;
	IAMStreamConfig *pConfig;
	VARIANT vName;
	VARIANT vPath;
	int pid;
	int vid;
};
#endif

#define SUVC_SUCCESS 0
#define SUVC_ERROR -1

#define FUNC_UVC        "uvc"

struct SSupport {
    std::string szFmt;
    std::vector<std::string> vtRes;
};

int suvc_init(SUvc *uvc, int vid, int pid);
int suvc_get_support(SUvc *uvc, std::vector<SSupport> &vtSupport);
void suvc_close(SUvc *uvc);
int suvc_list(void* pVid, void* pPid);
std::tuple<double, std::string> procFinalInfo(std::string info);

#endif