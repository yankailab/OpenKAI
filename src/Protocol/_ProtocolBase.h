#ifndef OpenKAI_src_Protocol__ProtocolBase_H_
#define OpenKAI_src_Protocol__ProtocolBase_H_

#include "../Base/common.h"
#include "../Base/_ThreadBase.h"
#include "../IO/_IOBase.h"

//0 PROTOCOL_BEGIN
//1 COMMAND
//2 PAYLOAD LENGTH
//3 Payload...

#define PROTOCOL_BEGIN 0xFE
#define PROTOCOL_N_HEADER 3

struct PROTOCOL_CMD
{
	int m_cmd;
	int m_nPayload;
	int m_iByte;
	uint8_t* m_pBuf;

	void init(int nB)
	{
		reset();

		if(nB<=0)
			nB = 256;
		m_pBuf = new uint8_t[nB];
	}

	void reset(void)
	{
		m_cmd = 0;
		m_nPayload = 0;
		m_iByte = 0;
	}
};

typedef void (*CallbackProtocol)(uint8_t* pCMD, void* pInst);

namespace kai
{

class _ProtocolBase: public _ThreadBase
{
public:
	_ProtocolBase();
	~_ProtocolBase();

	virtual bool init(void* pKiss);
	virtual bool start(void);
	virtual int check(void);
	virtual void draw(void);

	virtual bool readCMD(void);
	virtual void handleCMD(void);
	void setCallback(CallbackProtocol cb, void* pInst);

private:
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_ProtocolBase *) This)->update();
		return NULL;
	}

public:
	_IOBase*	m_pIO;
	uint8_t*	m_pBuf;
	int			m_nBuf;
	PROTOCOL_CMD m_recvMsg;
	uint64_t	m_nCMDrecv;

	CallbackProtocol m_pfCallback;
	void*		m_pfInst;
};

}
#endif
