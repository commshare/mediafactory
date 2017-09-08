#include "General.h"
#include "DSVideoGraph.h"
#include "baseclasses/mtype.h"

DSVideoGraph::DSVideoGraph()
    : video_cap_device_(NULL)
{

}

DSVideoGraph::~DSVideoGraph()
{
    Destroy();
}

int DSVideoGraph::Create(DSVideoCaptureDevice* videoCapDevice)
{
	video_cap_device_ = videoCapDevice;

	grabber_.CoCreateInstance( CLSID_SampleGrabber );
	if( !grabber_ )
	{
		return -1;
	}
	CComQIPtr<IBaseFilter, &IID_IBaseFilter> grabber_filter(grabber_);
	graph_builder_->AddFilter(grabber_filter, _T("Grabber"));

	// source filter
	source_filter_ = video_cap_device_->GetBaseFilter();

	CComQIPtr<IBaseFilter, &IID_IBaseFilter> cap_filter(source_filter_);
	graph_builder_->AddFilter(cap_filter, _T("Source"));
	video_cap_device_->SetOutPin(DSGraph::GetOutPin(cap_filter, 0 ));
	cap_filter.Release();

	AM_MEDIA_TYPE   mediaType;
	ZeroMemory(&mediaType, sizeof(AM_MEDIA_TYPE));  
	mediaType.subtype = MEDIASUBTYPE_RGB24;  
	mediaType.majortype = MEDIATYPE_Video;  
	mediaType.formattype = FORMAT_VideoInfo;  
	HRESULT hr = grabber_->SetMediaType(&mediaType);  
	if(FAILED(hr))
	{
		grabber_filter.Release();
		grabber_.Release();
		return -2;
	}

	hr = SetVideoFormat();
	if(FAILED(hr))
	{
		grabber_filter.Release();
		grabber_.Release();
		return -2;
	}

	// connect the capture device out pin to the grabber

	CComPtr<IPin> grabber_pin = DSGraph::GetInPin(grabber_filter, 0);
	grabber_filter.Release();

	hr = graph_builder_->Connect(video_cap_device_->GetOutPin(), grabber_pin);
	if(FAILED(hr))
	{
		grabber_pin.Release();
		grabber_.Release();
		return -3;
	}
	grabber_pin.Release();

    // buffering samples as they pass through
    grabber_->SetBufferSamples(TRUE);
    // not grabbing just one frame
    grabber_->SetOneShot(FALSE);
    // setting callback
    grabber_->SetCallback((ISampleGrabberCB*)(&grabber_callback_), 1);

    return 0;
}

void DSVideoGraph::Destroy()
{
    grabber_.Release();
}

HRESULT DSVideoGraph::SetVideoFormat()
{
	//////////////////////////////////////////////////////////////////////////////
	// 加入由 lWidth和lHeight设置的摄像头的宽和高 的功能，默认320*240
	// by flymanbox @2009-01-24
	//////////////////////////////////////////////////////////////////////////////
	IAMStreamConfig *iconfig = NULL;
	video_cap_device_->GetOutPin()->QueryInterface(IID_IAMStreamConfig, (void**)&iconfig);   

	AM_MEDIA_TYPE *pmt;
	if(iconfig->GetFormat(&pmt) !=S_OK)
	{
		//printf("GetFormat Failed ! \n");
		SAFE_RELEASE(iconfig);
		return -1;
	}

	VIDEOINFOHEADER *videoInfo = (VIDEOINFOHEADER*)(pmt->pbFormat);
	video_cap_device_->SetWidth(videoInfo->bmiHeader.biWidth);
	video_cap_device_->SetHeight(videoInfo->bmiHeader.biHeight);
	video_cap_device_->SetFrameRate( (10000000 / videoInfo->AvgTimePerFrame) );
	video_cap_device_->SetBitRate(videoInfo->dwBitRate);

// 	// 设置视频格式  
// 	VIDEOINFOHEADER vih;  
// 	AM_MEDIA_TYPE   mediaType;
// 	ZeroMemory(&mediaType, sizeof(AM_MEDIA_TYPE));  
// 	vih.bmiHeader.biWidth = nWidth;  
// 	vih.bmiHeader.biHeight = nHeight;  
// 	vih.bmiHeader.biSizeImage = nWidth*nHeight*3;  
// 	mediaType.pbFormat = (BYTE *)(&vih);  
// 	mediaType.cbFormat = sizeof(VIDEOINFOHEADER);  
// 	mediaType.subtype = MEDIASUBTYPE_RGB24;  
// 	mediaType.majortype = MEDIATYPE_Video;  
// 	mediaType.formattype = FORMAT_VideoInfo;  

	HRESULT hr = iconfig->SetFormat(pmt);
// 	if( FAILED( hr ) ){   
// 		printf("Fail to SetFormat!");   
// 		return hr;   
// 	}  

	DeleteMediaType( pmt );
	SAFE_RELEASE(iconfig);
	
	return 0;
}
