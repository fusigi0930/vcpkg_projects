#ifndef __UVC_UTILS_H__
#define __UVC_TUILS_H__

#include <vector>
#include <string>
#include <cstdint>
#include <tuple>

#if 1
#  include "libuvc/libuvc.h"
struct SUvc {
    uvc_context_t *ctx;
    uvc_device_t *dev;
    uvc_device_handle_t *devh;
};
#else
#  if (WIN_UVC_BACK == 0)
#    include <dshow.h>
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

#  elif (WIN_UVC_BACK == 1)
#    include <mfapi.h>
#    include <mfidl.h>
#    include <mfreadwrite.h>
#    include <mferror.h>
#    include <strmif.h>

struct SUvc {
	bool bCoinit;
	bool bMfStart;
	IMFAttributes *pAttr;
	IMFActivate **ppDevices;
	uint32_t count;
	int pid;
	int vid;
};
#  endif
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