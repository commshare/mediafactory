#ifndef _DS_GRABBER_CALLBACK_H_
#define _DS_GRABBER_CALLBACK_H_

#include "Lock.h"
#include "QueueBuffer.h"

#include <deque>
#define MAXMEDIADATASIZE (1024*1024)
class MEDIA_DATA
{
public:
	MEDIA_DATA()
	{
		mediadatalength = 0;
		memset(mediadata, 0, sizeof(mediadata));
		timestamp_ = ::GetTickCount();
	}
	int setdata(char *data,unsigned len, unsigned long timestamp)
	{
		mediadatalength = 0;
		if( !data || len > MAXMEDIADATASIZE )
			return -1;

		memset(mediadata, 0, sizeof(mediadata));
		memcpy(mediadata, data, len);
		mediadatalength = len;
		timestamp_ = timestamp;
		return 0;
	}
	char mediadata[MAXMEDIADATASIZE];
	unsigned mediadatalength;
	unsigned long timestamp_;
};

typedef std::deque<MEDIA_DATA> queueMEDIADATA;

class DSGrabberCallback : ISampleGrabberCB
{
public:
    DSGrabberCallback();

    ~DSGrabberCallback();

    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();
    STDMETHODIMP QueryInterface(REFIID riid, void** ppv);
    STDMETHODIMP SampleCB(double SampleTime, IMediaSample* pSample);
    STDMETHODIMP BufferCB(double dblSampleTime, BYTE* pBuffer, long lBufferSize);

    bool IsBufferAvailable();

	int GetBuffer(MEDIA_DATA &data);

    int BufferSize() { return buffer_size_; }

    __int64 Timestamp() { return timestamp_; }

    UINT FPSMeter() { return fps_meter_; }

    void ResetFPSMeter() { fps_meter_ = 0; }

private:
	volatile unsigned buffer_size_;
    __int64 timestamp_;
    bool is_buffer_available_;
    UINT fps_meter_;
    //base::Semaphore semaphore_;
    base::Lock lock_;
	MEDIA_DATA m_data;

	queueMEDIADATA m_mediaDataQueue;
};

#endif // _DS_GRABBER_CALLBACK_H_
