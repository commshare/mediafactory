#include "General.h"
#include "DSAudioCapture.h"

#include <map>

#include "DSAudioGraph.h"
#include "DSAudioCaptureDevice.h"
//#include "AudioEncoderThread.h"

DSAudioCapture::DSAudioCapture()
{
    ds_audio_graph_ = NULL;
    ds_audio_cap_device_ = NULL;
    //audio_encoder_thread_ = NULL;
}

DSAudioCapture::~DSAudioCapture()
{
    Destroy();
}

void DSAudioCapture::Destroy()
{
    SAFE_DELETE( ds_audio_graph_ );
    SAFE_DELETE( ds_audio_cap_device_ );
    //SAFE_DELETE( audio_encoder_thread_ );
}

void DSAudioCapture::Create(const CString& audioDeviceID,
    const DSAudioFormat& audioFormat, const CString& audioOutname)
{
    HRESULT hr;

    // audio
    ds_audio_graph_ = new DSAudioGraph;
    ds_audio_cap_device_ = new DSAudioCaptureDevice;
    //audio_encoder_thread_ = new AudioEncoderThread(ds_audio_graph_);

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

void DSAudioCapture::Start()
{
    //audio_encoder_thread_->Start();
    ds_audio_graph_->Start();
}

void DSAudioCapture::Stop()
{
    ds_audio_graph_->Stop();
    //audio_encoder_thread_->Stop();
    //audio_encoder_thread_->Join();
}

int DSAudioCapture::GetBuffer(MEDIA_DATA &data)
{
	return ds_audio_graph_->GetBuffer(data);
}

HRESULT DSAudioCapture::SetAudioFormat( DWORD dwPreferredSamplesPerSec, WORD wPreferredBitsPerSample, WORD nPreferredChannels )
{
    Stop();

    ds_audio_graph_->Destroy();

    ds_audio_cap_device_->SetPreferredSamplesPerSec( dwPreferredSamplesPerSec );
    ds_audio_cap_device_->SetPreferredBitsPerSample( wPreferredBitsPerSample );
    ds_audio_cap_device_->SetPreferredChannels( nPreferredChannels );

    HRESULT hr = ds_audio_graph_->Create( ds_audio_cap_device_ );
    if( FAILED( hr ) )
    {
        return hr;
    }

    Start();

    return 0;
}

int DSAudioCapture::BufferSize()
{
	return ds_audio_graph_->BufferSize();
}

