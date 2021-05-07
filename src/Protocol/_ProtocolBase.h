#ifndef OpenKAI_src_Protocol__ProtocolBase_H_
#define OpenKAI_src_Protocol__ProtocolBase_H_

#include "../Base/_ModuleBase.h"
#include "../IO/_IOBase.h"

//0 PROTOCOL_BEGIN
//1 COMMAND
//2 PAYLOAD LENGTH
//3 Payload...

#define PB_BEGIN 0xFE
#define PB_N_HDR 3

struct PROTOCOL_CMD
{
	int m_cmd;
	int m_nPayload;
	int m_iB;
	uint8_t* m_pB;
    int m_nB;

	void init(int nB)
	{
		reset();

		if(nB<=0)
			nB = 256;
        m_nB = nB;
		m_pB = new uint8_t[m_nB];
	}

	void reset(void)
	{
		m_cmd = 0;
		m_nPayload = 0;
		m_iB = 0;
	}
};

typedef void (*CallbackProtocol)(uint8_t* pCMD, void* pInst);

namespace kai
{

class _ProtocolBase: public _ModuleBase
{
public:
	_ProtocolBase();
	~_ProtocolBase();

	virtual bool init(void* pKiss);
	virtual bool start(void);
	virtual int check(void);
	virtual void console(void* pConsole);

	virtual bool readCMD(void);
	virtual void handleCMD(void);
	void setCallback(CallbackProtocol cb, void* pInst);

private:
	void update(void);
	static void* getUpdate(void* This)
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
