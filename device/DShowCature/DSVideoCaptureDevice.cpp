#include "General.h"
#include "DSVideoCaptureDevice.h"

DSVideoCaptureDevice::DSVideoCaptureDevice()
{
	width_ = height_ = framepersec_ = 0;
}

DSVideoCaptureDevice::~DSVideoCaptureDevice()
{

}

HRESULT DSVideoCaptureDevice::Create(const CString& comObjID)
{
    return DSCaptureDevice::Create(comObjID);
}
