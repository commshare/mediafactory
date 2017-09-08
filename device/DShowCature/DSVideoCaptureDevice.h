#ifndef _DS_VIDEO_CAPTURE_DEVICE_H_
#define _DS_VIDEO_CAPTURE_DEVICE_H_

#include "DSCaptureDevice.h"

class DSVideoCaptureDevice : public DSCaptureDevice
{
public:
    DSVideoCaptureDevice();

    ~DSVideoCaptureDevice();

    HRESULT Create(const CString& comObjID);

    // -----------------------------------------------------------------------
    // seto¨ªget¨ºy?Y3¨¦?¡À
	void SetWidth(DWORD width) { width_ = width; }
	DWORD GetWidth() { return width_; }

	void SetHeight(DWORD height) { height_ = height; }
	DWORD GetHeight() { return height_; }

	void SetFrameRate(DWORD fps) { framepersec_ = fps; }
	DWORD GetFrameRate() { return framepersec_; }

	void SetBitRate(DWORD bps) { bitrate_ = bps; }
	DWORD GetBitRate() { return bitrate_; }
private:
    DWORD width_;
    DWORD height_;
    DWORD framepersec_;
	DWORD bitrate_;
};

#endif // _DS_VIDEO_CAPTURE_DEVICE_H_
