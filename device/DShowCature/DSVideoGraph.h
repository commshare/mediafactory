#ifndef _DS_VIDEO_GRAPH_H_
#define _DS_VIDEO_GRAPH_H_

#include "DSGraph.h"
#include "DSVideoCaptureDevice.h"

class DSVideoGraph : public DSGraph
{
public:
    DSVideoGraph();

    ~DSVideoGraph();

    int Create(DSVideoCaptureDevice* videoCapDevice);

    void Destroy();

	DSVideoCaptureDevice* GetCapDevie(){ return video_cap_device_; }

	DWORD GetWidth() { return video_cap_device_->GetWidth(); }

	DWORD GetHeight() { return video_cap_device_->GetHeight(); }

	DWORD GetFrameRate() { return video_cap_device_->GetFrameRate(); }

	DWORD GetBitRate() { return video_cap_device_->GetBitRate(); }
private:
    HRESULT SetVideoFormat();

private:
    DSVideoCaptureDevice* video_cap_device_;
};

#endif // _DS_VIDEO_GRAPH_H_
