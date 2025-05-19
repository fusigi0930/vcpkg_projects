#include "uvc_utils.h"

#include <map>
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>

static std::string genRes(int w, int h) {
    uint32_t r = static_cast<uint32_t>(w) << 16 | static_cast<uint32_t>(h);
#if 0
    std::map<uint32_t, std::string> mapRes;
    mapRes[0x0f000870]="4k";
    mapRes[0x0a0005a0]="2k";
    mapRes[0x07800438]="1080";
    mapRes[0x050002d0]="720";
    mapRes[0x02800168]="360";
    mapRes[0x00a00078]="120";
    mapRes[0x00b00090]="144";
    mapRes[0x014000f0]="240";
    mapRes[0x01e0010e]="270";

    std::map<uint32_t, std::string>::iterator pFind = mapRes.find(r);
    if (pFind == mapRes.end()) {
        std::cerr << "not support resolution!" << " " << w << ", " << h << ", "
        << std::hex << r << std::endl;
        std::stringstream s;
        s << std::hex << r;
        return s.str();
    }

    return pFind->second;
#else
    std::stringstream s;
    s << std::hex << r;
    return s.str();
#endif
}

static std::string genFinalInfo(double rate, std::string res) {
	std::stringstream s;
	s << std::dec << rate << "_" << res;
	return s.str();
}

#if !defined(_WIN32)
int suvc_init(SUvc *uvc, int vid, int pid) {
    if (nullptr == uvc)
        return SUVC_ERROR;

    int res = uvc_init(&uvc->ctx, nullptr);
    if (0 > res) {
        std::cerr << "uvc_init failed" << std::endl;
        return SUVC_ERROR;
    }
 
    res = uvc_find_device(uvc->ctx, &uvc->dev, vid, pid, nullptr);
    if (0 > res) {
        std::cerr << "uvc_find_device failed" << std::endl;
        return SUVC_ERROR;
    }

    res = uvc_open(uvc->dev, &uvc->devh);
    if (0 > res) {
        std::cerr << "uvc_open failed" << std::endl << uvc->devh;
        return SUVC_ERROR;
    }

    return SUVC_SUCCESS;
}

int suvc_get_support(SUvc *uvc, std::vector<SSupport> &vtSupport) {
    if (nullptr == uvc || nullptr == uvc->devh)
        return SUVC_ERROR;

    vtSupport.clear();
    
    uvc_format_desc_t *format_desc = const_cast<uvc_format_desc_t*>(uvc_get_format_descs(uvc->devh));
    while (nullptr != format_desc) {
        SSupport sup;
        switch(format_desc->bDescriptorSubtype) {
            case UVC_VS_FORMAT_MJPEG:
                sup.szFmt = "mjpeg";
                break;
            default:
                std::cout << "type: " << format_desc->bDescriptorSubtype << std::endl;
                sup.szFmt = "yuv422";
                break;
        }
        std::cout << "get format: " << sup.szFmt << std::endl;
        uvc_frame_desc_t *frame_desc = format_desc->frame_descs;
        while (nullptr != frame_desc) {
			double rate = 10000000.0 / frame_desc->intervals[0];
            std::string r = genFinalInfo(rate, genRes(frame_desc->wWidth, frame_desc->wHeight));
            if (!r.empty()) {
                sup.vtRes.push_back(r);
            }
            frame_desc = frame_desc->next;
        }
        vtSupport.push_back(sup);
        format_desc = format_desc->next;
    }

    return SUVC_SUCCESS;
}

void suvc_close(SUvc *uvc) {
    if (nullptr == uvc)
        return;

    if (uvc->devh) {
        uvc_close(uvc->devh);
        uvc->devh = nullptr;
    }
    if (uvc->dev) {
        uvc_unref_device(uvc->dev);
        uvc->dev = nullptr;
    }
    if (uvc->ctx) {
        uvc_exit(uvc->ctx);
        uvc->ctx = nullptr;
    }
}

#else
#if (WIN_UVC_BACK == 0)
int suvc_init(SUvc *uvc, int vid, int pid) {
	if (nullptr == uvc) {
		std::cerr << "uvc pointer is null" << std::endl;
		return SUVC_ERROR;
	}
	HRESULT hr = CoInitialize(nullptr);
	if (FAILED(hr)) {
		std::cerr << "com init failed" << std::endl;
		return SUVC_ERROR;
	}
	
	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
                          IID_ICreateDevEnum, reinterpret_cast<void**>(&uvc->pDevEnum));

	if (FAILED(hr)) {
		std::cerr << "create instance failed" << std::endl;
		return SUVC_ERROR;
	}

	hr = uvc->pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &uvc->pEnum, 0);
	if (FAILED(hr)) {
		std::cerr << "get class enum failed" << std::endl;
		return SUVC_ERROR;
	}


	uvc->vid = vid;
	uvc->pid = pid;

	return SUVC_SUCCESS;
}

int suvc_get_support(SUvc *uvc, std::vector<SSupport> &vtSupport) {
	if (nullptr == uvc || nullptr == uvc->pEnum) {
		std::cerr << __FUNCTION__ << " uvc pointer is null" << std::endl;
		return SUVC_ERROR;	
	}

	while (uvc->pEnum->Next(1, &uvc->pMoniker, nullptr) == S_OK) {
		HRESULT hr = uvc->pMoniker->BindToStorage(0, 0, IID_IPropertyBag, reinterpret_cast<void**>(&uvc->pPropBag));
		if (FAILED(hr)) {
			std::cerr << __FUNCTION__ << " bind to storage failed, find next!!" << std::endl;
			continue;
		}
		VARIANT vName, vPath;
		VariantInit(&vName);
		VariantInit(&vPath);
		uvc->pPropBag->Read(L"FriendlyName", &vName, 0);
		uvc->pPropBag->Read(L"DevicePath", &vPath, 0);

		std::wstringstream s;
		s << L"vid_" << std::hex << std::setw(4) << std::setfill(L'0') << uvc->vid
		  << std::setw(0) << L"&pid_" << std::setw(4) << std::setfill(L'0') << uvc->pid;

		if (vPath.bstrVal != nullptr && nullptr == wcsstr(vPath.bstrVal, s.str().c_str())) {
			std::cerr << "vid, pid doesn't match!, next!" << std::endl;
			continue;
		}

		hr = uvc->pMoniker->BindToObject(0, 0, IID_IBaseFilter, reinterpret_cast<void**>(&uvc->pFilter));
		if (FAILED(hr)) {
			std::cerr << __FUNCTION__ << " bind to object failed, find next!!" << std::endl;
			continue;
		}

		hr = uvc->pFilter->EnumPins(&uvc->pEnumPins);
		if (FAILED(hr)) {
			std::cerr << __FUNCTION__ << " enum pins failed, find next!! " << std::hex <<hr << std::dec << std::endl;
			continue;
		}
		
		while (uvc->pEnumPins->Next(1, &uvc->pPin, nullptr) == S_OK) {
			hr = uvc->pPin->QueryInterface(IID_IAMStreamConfig, reinterpret_cast<void**>(&uvc->pConfig));
			if (FAILED(hr)) {
				std::cerr << __FUNCTION__ << " query interface failed, find next!! " << std::hex <<hr << std::dec << std::endl;
				continue;
			}

			int count = 0, size = 0;
			hr = uvc->pConfig->GetNumberOfCapabilities(&count, &size);
			if (FAILED(hr) || size != sizeof(VIDEO_STREAM_CONFIG_CAPS)) {
				std::cerr << __FUNCTION__ << " get numbers failed, find next!!" << std::endl;
				continue;
			}

			std::cout << "capacities number: " << std::dec << count << std::endl;
			for (int i=0; i<count; i++) {
				AM_MEDIA_TYPE *pmt = nullptr;
				VIDEO_STREAM_CONFIG_CAPS caps;
				hr = uvc->pConfig->GetStreamCaps(i, &pmt, reinterpret_cast<BYTE*>(&caps));
				if (FAILED(hr)) {
					std::cerr << __FUNCTION__ << " get stream caps failed, find next!!" << std::endl;
					continue;
				}
				SSupport sup;
				//std::cout << "type: " << std::hex << pmt->subtype.Data1 << std::dec << std::endl;
				//std::cout << "yuv: " << std::hex << MEDIASUBTYPE_YUY2.Data1 << std::dec << std::endl;
				//std::cout <<  "mjpg: " << std::hex << MEDIASUBTYPE_MJPG.Data1 << std::dec << std::endl;

				if (FORMAT_VideoInfo == pmt->formattype) {
					VIDEOINFOHEADER *info = reinterpret_cast<VIDEOINFOHEADER*>(pmt->pbFormat);
					if (MEDIASUBTYPE_YUY2.Data1 == pmt->subtype.Data1) {
					//if (MAKEFOURCC('Y','U','Y','2') == info->bmiHeader.biCompression) {
						//std::cout << "get type yuv422" << std::endl;
						sup.szFmt = "yuv422";
					}
					else if (MEDIASUBTYPE_MJPG.Data1 == pmt->subtype.Data1) {
					//else if (MAKEFOURCC('M','J','P','G') == info->bmiHeader.biCompression) {
						//std::cout << "get type mjpeg" << std::endl;
						sup.szFmt = "mjpeg";
					}
					else {
						continue;
					}
					//std::cout << std::dec << "res: " << info->bmiHeader.biWidth << "x" << info->bmiHeader.biHeight << " " << genRes(static_cast<int>(info->bmiHeader.biWidth), static_cast<int>(info->bmiHeader.biHeight)) << std::endl;
					double rate = 0.0;
					if (0 != info->AvgTimePerFrame)
						rate = 10000000.0 / static_cast<double>(info->AvgTimePerFrame);
					sup.vtRes.push_back(genFinalInfo(rate, genRes(static_cast<int>(info->bmiHeader.biWidth), static_cast<int>(info->bmiHeader.biHeight))));
					vtSupport.push_back(sup);
				}
				if (pmt) {
					if (0 != pmt->cbFormat) {
						CoTaskMemFree(pmt->pbFormat);
						pmt->cbFormat = 0;
						pmt->pbFormat = nullptr;
					}
					if (pmt->pUnk) {
						pmt->pUnk->Release();
						pmt->pUnk = nullptr;
					}
					CoTaskMemFree(pmt);
				}
			}

			uvc->pPin->Release();
			uvc->pPin = nullptr;
		}
		VariantClear(&vPath);
		VariantClear(&vName);
	}
	return 0;
}

void suvc_close(SUvc *uvc) {
	if (nullptr == uvc) {
		return;
	}
	if (uvc->pConfig) {
		uvc->pConfig->Release();
		uvc->pConfig = nullptr;
	}
	if (uvc->pPin) {
		uvc->pPin->Release();
		uvc->pPin = nullptr;
	}
	if (uvc->pEnumPins) {
		uvc->pEnumPins->Release();
		uvc->pEnumPins = nullptr;
	}
	if (uvc->pFilter) {
		uvc->pFilter->Release();
		uvc->pFilter = nullptr;
	}
	if (uvc->pPropBag) {
		uvc->pPropBag->Release();
		uvc->pPropBag = nullptr;
	}
	if (uvc->pMoniker) {
		uvc->pMoniker->Release();
		uvc->pMoniker = nullptr;
	}
	if (uvc->pEnum) {
		uvc->pEnum->Release();
		uvc->pEnum = nullptr;
	}
	if (uvc->pDevEnum) {
		uvc->pDevEnum->Release();
		uvc->pDevEnum = nullptr;
	}

	CoUninitialize();
}
#elif (WIN_UVC_BACK == 1)
int suvc_init(SUvc *uvc, int vid, int pid) {
	if (nullptr == uvc) {
		std::cerr << "uvc pointer is null" << std::endl;
		return SUVC_ERROR;
	}
	uvc->pid = pid;
	uvc->vid = vid;
	HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	if (FAILED(hr)) {
		std::cerr << "co initailize failed" << std::endl;
		return SUVC_ERROR;
	}
	uvc->bCoinit = true;
	hr = MFStartup(MF_VERSION, MFSTARTUP_FULL);
	if (FAILED(hr)) {
		std::cerr << "mfstartup failed: " << std::hex << hr << std::dec << std::endl;
		return SUVC_ERROR;
	}
	uvc->bMfStart = true;
	hr = MFCreateAttributes(&uvc->pAttr, 1);
	if (FAILED(hr)) {
		std::cerr << "mf create attributes failed: " << std::hex << hr << std::dec << std::endl;
		return SUVC_ERROR;
	}

	uvc->pAttr->SetUINT32(MF_SOURCE_READER_DISCONNECT_MEDIASOURCE_ON_SHUTDOWN, TRUE);
	uvc->pAttr->SetUINT32(MF_SOURCE_READER_ENABLE_ADVANCED_VIDEO_PROCESSING, FALSE);
	hr = uvc->pAttr->SetGUID(
		MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
        MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
	if (FAILED(hr)) {
		std::cerr << "attributes set guid failed: " << std::hex << hr << std::dec << std::endl;
		return SUVC_ERROR;
	}

	hr = MFEnumDeviceSources(uvc->pAttr, &uvc->ppDevices, &uvc->count);
	if (FAILED(hr)) {
		std::cerr << "enum devices failed: " << std::hex << hr << std::dec << std::endl;
		return SUVC_ERROR;
	}
	if (0 >= uvc->count) {
		std::cerr << "no uvc device found!" << std::endl;
		return SUVC_ERROR;
	}

	return SUVC_SUCCESS;
}

int suvc_get_support(SUvc *uvc, std::vector<SSupport> &vtSupport) {
	if (nullptr == uvc) {
		std::cerr << "uvc pointer is null" << std::endl;
		return SUVC_ERROR;
	}

	for (uint32_t i=0; i<uvc->count; i++) {
		WCHAR *link = nullptr;
		uint32_t num = 0;
		HRESULT hr = uvc->ppDevices[i]->GetAllocatedString(
			MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK,
			&link,
			&num);
		if (FAILED(hr) || nullptr == link) {
			std::cerr << "get allocate string failed: " << std::hex << hr << std::dec << std::endl;
			continue;
		}

		std::wstringstream s;
		s << L"vid_" << std::hex << std::setw(4) << std::setfill(L'0') << uvc->vid
		  << std::setw(0) << L"&pid_" << std::setw(4) << std::setfill(L'0') << uvc->pid;

		if (link != nullptr && nullptr == wcsstr(link, s.str().c_str())) {
			std::cerr << "vid, pid doesn't match!, next!" << std::endl;
			CoTaskMemFree(link);
			continue;
		}
		CoTaskMemFree(link);

		IMFMediaSource* pSource = nullptr;
		IMFSourceReader* pReader = nullptr;
		hr = uvc->ppDevices[i]->ActivateObject(IID_PPV_ARGS(&pSource));
		if (FAILED(hr)) {
			std::cerr << "activate object failed: " << std::hex << hr << std::dec << std::endl;
			continue;
		}

		IAMVideoProcAmp* pProcAmp = nullptr;
		hr = pSource->QueryInterface(IID_PPV_ARGS(&pProcAmp));
		if (SUCCEEDED(hr)) {
			long value = 1;
			pProcAmp->Set(VideoProcAmp_BacklightCompensation, value, VideoProcAmp_Flags_Manual);
			pProcAmp->Release();
		}

		hr = MFCreateSourceReaderFromMediaSource(pSource, uvc->pAttr, &pReader);
		if (FAILED(hr)) {
			std::cerr << "create reader failed: " << std::hex << hr << std::dec << std::endl;
			pSource->Release();
			continue;
		}

		DWORD index=0;
		do {
			IMFMediaType* pType = nullptr;
            hr = pReader->GetNativeMediaType(
                static_cast<DWORD>(MF_SOURCE_READER_FIRST_VIDEO_STREAM),
                index,
                &pType);

			if (MF_E_NO_MORE_TYPES == hr) {
				break;
			}
			else if (FAILED(hr)) {
				index++;
				continue;
			}

			GUID Type = {0};
			pType->GetGUID(MF_MT_MAJOR_TYPE, &Type);
			if (MFMediaType_Video != Type) {
				pType->Release();
				index++;
				continue;
			}

			pType->GetGUID(MF_MT_SUBTYPE, &Type);
			SSupport sup;
			if (MFVideoFormat_MJPG == Type) {
				sup.szFmt = "mjpeg";
			}
			else if (MFVideoFormat_YUY2 == Type) {
				sup.szFmt = "yuv422";
			}

			UINT32 w=0, h=0, numerator=0, denominator=0;
			MFGetAttributeSize(pType, MF_MT_FRAME_SIZE, &w, &h);
			MFGetAttributeRatio(pType, MF_MT_FRAME_RATE, &numerator, &denominator);

			double rate = static_cast<double>(numerator) / static_cast<double>(denominator);
			sup.vtRes.push_back(genFinalInfo(rate, genRes(static_cast<int>(w), static_cast<int>(h))));
			vtSupport.push_back(sup);

			pType->Release();
			index++;
		} while (SUCCEEDED(hr));
	}
	return SUVC_SUCCESS;
}

void suvc_close(SUvc *uvc) {
	if (nullptr == uvc) {
		return;
	}
	if (uvc->ppDevices) {
		for (uint32_t i=0; i<uvc->count; i++) {
			uvc->ppDevices[i]->Release();
			uvc->ppDevices[i] = nullptr;
		}
		CoTaskMemFree(uvc->ppDevices);
		uvc->ppDevices = nullptr;
	}
	if (uvc->pAttr) {
		uvc->pAttr->Release();
		uvc->pAttr = nullptr;
	}
	if (uvc->bMfStart) {
		MFShutdown();
		uvc->bMfStart = false;
	}
	if (uvc->bCoinit) {
		CoUninitialize();
		uvc->bCoinit = false;
	}
}

#endif
#endif

std::tuple<double, std::string> procFinalInfo(std::string info) {
	if (info.empty() || std::string::npos == info.find("_")) {
		return std::tuple<double, std::string>(0.0, "");
	}
    auto pos = info.find("_");
    std::string l=info.substr(0, pos);
    std::string r=info.substr(pos+1);

	std::stringstream s;
	s << l;
	double rate;
	s >> rate;
	return std::tuple<double, std::string>(rate, r);
}

int suvc_list(void *pVid, void *pPid) {
    int vid = static_cast<int>(reinterpret_cast<uint64_t>(pVid));
    int pid = static_cast<int>(reinterpret_cast<uint64_t>(pPid));
    SUvc uvc = {0}; 
    if (SUVC_SUCCESS != suvc_init(&uvc, vid, pid)) {
        std::cerr << "init failed" << std::endl;
        return -1;
    }
    std::vector<SSupport> vtSupport;
    suvc_get_support(&uvc, vtSupport);
    suvc_close(&uvc);
    for (std::vector<SSupport>::iterator p=vtSupport.begin(); p != vtSupport.end(); p++) {
        std::cout << "format: " << p->szFmt << std::endl;
        for (std::vector<std::string>::iterator r=p->vtRes.begin(); r != p->vtRes.end(); r++) {
			double rate;
			std::string res;
			std::tie(rate, res) = procFinalInfo(*r);
            std::stringstream s;
            s << std::hex << res;
            uint32_t mix_res;
            s >> mix_res;
            int w = mix_res >> 16;
            int h = mix_res & 0xffff;
            std::cout << "\t res: " << std::dec << w << ", " << h << ", rate: " << rate << std::endl;
        }
    }
    return SUVC_SUCCESS;
}