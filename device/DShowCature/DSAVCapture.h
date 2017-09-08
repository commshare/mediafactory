#ifndef _DS_AVCAPTURE_H_
#define _DS_AVCAPTURE_H_

#include "DSAudioCapture.h"
#include "DSVideoCapture.h"

class DSAudioGraph;
class DSAudioCaptureDevice;
class DSVideoGraph;
class DSVideoCaptureDevice;

class DSAVCapture : public DSCapture
{
public:
    DSAVCapture();

    ~DSAVCapture();


	void Create(const CString& audioDeviceID, const DSAudioFormat& audioFormat, 
			const CString& videoDeviceID, const DSVideoFormat& videoFormat);
	void Destroy();

	void Start();
	void Stop();

	int GetAudioBuffer(MEDIA_DATA &data);
	int GetVideoBuffer(MEDIA_DATA &data);	

	DSAudioGraph* GetAudioGraph() { return ds_audio_graph_; }
	DSVideoGraph* GetvideoGraph() { return ds_video_graph_; }
private:
	void _CreateAudioCapture(const CString& audioDeviceID, const DSAudioFormat& audioFormat);

	void _CreateVideoCapture(const CString& audioDeviceID, const DSVideoFormat& videoFormat);

private:

	DSVideoCaptureDevice*	ds_video_cap_device_;

	DSVideoGraph*			ds_video_graph_;

	DSAudioCaptureDevice*	ds_audio_cap_device_;

	DSAudioGraph*			ds_audio_graph_;
};

#endif // _DS_CAPTURE_H_
