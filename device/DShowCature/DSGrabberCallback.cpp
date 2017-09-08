#include "General.h"
#include "DSGrabberCallback.h"

DSGrabberCallback::DSGrabberCallback()
    : is_buffer_available_(false)
{
	buffer_size_ = 0;
}

DSGrabberCallback::~DSGrabberCallback()
{
	m_mediaDataQueue.clear();
}

STDMETHODIMP_(ULONG) DSGrabberCallback::AddRef()
{
    return 2;
}

STDMETHODIMP_(ULONG) DSGrabberCallback::Release()
{
    return 1;
}

STDMETHODIMP DSGrabberCallback::QueryInterface(REFIID riid, void ** ppv)
{
    if( riid == IID_ISampleGrabberCB || riid == IID_IUnknown ) 
    {
        *ppv = (void *) static_cast<ISampleGrabberCB*> ( this );
        return NOERROR;
    }    

    return E_NOINTERFACE;
}

STDMETHODIMP DSGrabberCallback::SampleCB(double SampleTime, IMediaSample* pSample)
{
    return 0;
}

STDMETHODIMP DSGrabberCallback::BufferCB(double dblSampleTime, BYTE *pBuffer, long lBufferSize)
{
    if (!pBuffer)
    {
        return E_POINTER;
    }

    //semaphore_.Wait();
//    base::AutoLock al(lock_);

    timestamp_ = ::GetTickCount();

	m_data.setdata((char*)pBuffer, lBufferSize, ::GetTickCount() );
	if( m_data.mediadatalength > 0 )
	{
		m_mediaDataQueue.push_back(m_data);
		buffer_size_++;
	}

    is_buffer_available_ = true;

    fps_meter_++;

    //semaphore_.Post();

    return 0;
}

bool DSGrabberCallback::IsBufferAvailable()
{
    return is_buffer_available_;
}

int DSGrabberCallback::GetBuffer(MEDIA_DATA &data)
{
	//semaphore_.Wait();
//	base::AutoLock al(lock_);

	if( buffer_size_ > 1 )
	{
		data = m_mediaDataQueue.front();
		m_mediaDataQueue.pop_front();
		buffer_size_--;

		return 0;
	}

	return -1;
}

