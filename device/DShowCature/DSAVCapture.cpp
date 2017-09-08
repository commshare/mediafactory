#include "General.h"
#include "DSAVCapture.h"

#include "DSAudioGraph.h"
#include "DSAudioCaptureDevice.h"
#include "DSVideoGraph.h"
#include "DSVideoCaptureDevice.h"

DSAVCapture::DSAVCapture()
{
	ds_audio_graph_ = NULL;
	ds_audio_cap_device_ = NULL;

	ds_video_graph_ = NULL;
	ds_video_cap_device_ = NULL;
}

DSAVCapture::~DSAVCapture()
{
	Destroy();
}

void DSAVCapture::Destroy()
{
	SAFE_DELETE( ds_audio_graph_ );
	SAFE_DELETE( ds_audio_cap_device_ );

	SAFE_DELETE( ds_video_graph_ );
	SAFE_DELETE( ds_video_cap_device_ );
}


void DSAVCapture::Create( const CString& audioDeviceID, const DSAudioFormat& audioFormat, const CString& videoDeviceID, const DSVideoFormat& videoFormat )
{
	_CreateAudioCapture(audioDeviceID, audioFormat);

	_CreateVideoCapture(videoDeviceID, videoFormat);
}

void DSAVCapture::_CreateAudioCapture(const CString& audioDeviceID,
							const DSAudioFormat& audioFormat)
{
	HRESULT hr;

	// audio
	ds_audio_graph_ = new DSAudioGraph;
	ds_audio_cap_device_ = new DSAudioCaptureDevice;

	hr = ds_audio_cap_device_->Create(audioDeviceID);
	if( FAILED( hr ) )
	{
		SAFE_DELETE( ds_audio_cap_device_ );
		SAFE_DELETE( ds_audio_graph_ );
	}
	if( ds_audio_cap_device_ )
	{
		ds_audio_cap_device_->SetPreferredSamplesPerSec(audioFormat.samples_per_sec);
		ds_audio_cap_device_->SetPreferredBitsPerSample(audioFormat.bits_per_sample);
		ds_audio_cap_device_->SetPreferredChannels(audioFormat.channels);

		HRESULT hr = ds_audio_graph_->Create( ds_audio_cap_device_ );
		if( FAILED( hr ) )
		{

		}
		//else
		//{
		//    audio_encoder_thread_->SetOutputFilename(CStringToString(audioOutname));
		//}
	}
}

void DSAVCapture::_CreateVideoCapture(const CString& videoDeviceID,
							const DSVideoFormat& videoFormat)
{
	HRESULT hr;

	// video
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

void DSAVCapture::Start()
{
	ds_audio_graph_->Start();
}

void DSAVCapture::Stop()
{
	ds_audio_graph_->Stop();
}

int DSAVCapture::GetAudioBuffer(MEDIA_DATA &data)
{
	return ds_audio_graph_->GetBuffer(data);
}

int DSAVCapture::GetVideoBuffer(MEDIA_DATA &data)
{
	return ds_video_graph_->GetBuffer(data);
}