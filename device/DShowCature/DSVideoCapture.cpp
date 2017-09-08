#include "General.h"
#include "DSVideoCapture.h"

#include <map>

#include "DSVideoGraph.h"
#include "DSVideoCaptureDevice.h"
//#include "AudioEncoderThread.h"

DSVideoCapture::DSVideoCapture()
{
    ds_video_graph_ = NULL;
    ds_video_cap_device_ = NULL;
    //audio_encoder_thread_ = NULL;
}

DSVideoCapture::~DSVideoCapture()
{
    Destroy();
}

void DSVideoCapture::Destroy()
{
    SAFE_DELETE( ds_video_graph_ );
    SAFE_DELETE( ds_video_cap_device_ );
    //SAFE_DELETE( audio_encoder_thread_ );
}

void DSVideoCapture::Create(const CString& videoDeviceID,
    const DSVideoFormat& videoFormat, const CString& audioOutname)
{
    HRESULT hr;

    // audio

    ds_video_graph_ = new DSVideoGraph;
    ds_video_cap_device_ = new DSVideoCaptureDevice;
    //audio_encoder_thread_ = new AudioEncoderThread(ds_audio_graph_);

    hr = ds_video_cap_device_->Create(videoDeviceID);
    if( FAILED( hr ) )
    {
        SAFE_DELETE( ds_video_cap_device_ );
        SAFE_DELETE( ds_video_graph_ );
    }
    if( ds_video_cap_device_ )
    {
         ds_video_cap_device_->SetWidth(videoFormat.width);
         ds_video_cap_device_->SetHeight(videoFormat.height);
		 ds_video_cap_device_->SetFrameRate(videoFormat.fps);
		 ds_video_cap_device_->SetBitRate(videoFormat.bps);

        HRESULT hr = ds_video_graph_->Create( ds_video_cap_device_ );
        if( FAILED( hr ) )
        {
            
        }
        //else
        //{
        //    audio_encoder_thread_->SetOutputFilename(CStringToString(audioOutname));
        //}
    }
}

void DSVideoCapture::Start()
{
    //audio_encoder_thread_->Start();
    ds_video_graph_->Start();
}

void DSVideoCapture::Stop()
{
    ds_video_graph_->Stop();
    //audio_encoder_thread_->Stop();
    //audio_encoder_thread_->Join();
}

int DSVideoCapture::GetBuffer(MEDIA_DATA &data)
{
	return ds_video_graph_->GetBuffer(data);
}

int DSVideoCapture::BufferSize()
{
	return ds_video_graph_->BufferSize();
}


