#ifndef _DS_VIDEOCAPTURE_H_
#define _DS_VIDEOCAPTURE_H_

#include "dscapture.h"

class DSVideoGraph;
class DSVideoCaptureDevice;
//class AudioEncoderThread;

struct DSVideoFormat
{
    unsigned int width;
    unsigned int height;
    unsigned int fps;
	unsigned int bps;
};

class DSVideoCapture : public DSCapture
{
public:
    DSVideoCapture();

    ~DSVideoCapture();

    void Create(const CString& audioDeviceID, const DSVideoFormat& audioFormat, const CString& audioOutname);

    void Destroy();

    void Start();

    void Stop();

	int GetBuffer(MEDIA_DATA &data);	
	int BufferSize();	

    DSVideoGraph* GetVideoGraph() { return ds_video_graph_; }

private:

    DSVideoCaptureDevice*	ds_video_cap_device_;

    DSVideoGraph*			ds_video_graph_;

    //AudioEncoderThread* audio_encoder_thread_;
};

#endif // _DS_VIDEOCAPTURE_H_
