#ifndef _DS_AUDIOCAPTURE_H_
#define _DS_AUDIOCAPTURE_H_

#include "dscapture.h"

class DSAudioGraph;
class DSAudioCaptureDevice;
//class AudioEncoderThread;

struct DSAudioFormat
{
    unsigned int samples_per_sec;
    unsigned int bits_per_sample;
    unsigned int channels;
};

class DSAudioCapture : virtual public DSCapture
{
public:
    DSAudioCapture();

    ~DSAudioCapture();

    void Create(const CString& audioDeviceID, const DSAudioFormat& audioFormat, const CString& audioOutname);

    void Destroy();


    HRESULT	SetAudioFormat(DWORD dwPreferredSamplesPerSec,
        WORD wPreferredBitsPerSample, WORD nPreferredChannels);

    void Start();

    void Stop();

	int GetBuffer(MEDIA_DATA &data);
	int BufferSize();

    DSAudioGraph* GetAudioGraph() { return ds_audio_graph_; }

private:

    DSAudioCaptureDevice*	ds_audio_cap_device_;

    DSAudioGraph*			ds_audio_graph_;

    //AudioEncoderThread* audio_encoder_thread_;
};

#endif // _DS_AUDIOCAPTURE_H_
