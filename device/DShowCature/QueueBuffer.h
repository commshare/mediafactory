#ifndef _QUEUEBUFFER_H_
#define _QUEUEBUFFER_H_

//#include "Lock.h"

class CQueueBuffer
{
public:
	CQueueBuffer(int nBufferLen);
	virtual ~CQueueBuffer(void);

public:
	int Read(char*pBuffer,int nLen);
	void Write(char*pData,int nLen);
	int GetFilledSize(void);
	int GetFreeSize(void);

protected:
	void MovePointer(char**p,int nOffset);

protected:
	//base::Lock		m_csQueueBuffer;
	char*			m_pBufferHead;
	char*			m_pBufferTail;
	int				m_nBufferLen;
	char*			m_pFront;
	char*			m_pBack;
	bool			m_bEmpty;
};

#endif