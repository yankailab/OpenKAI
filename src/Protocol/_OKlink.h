#ifndef OpenKAI_src_Protocol__OKlink_H_
#define OpenKAI_src_Protocol__OKlink_H_

#include "../Base/common.h"
#include "../Base/_ThreadBase.h"
#include "../IO/_IOBase.h"

//0 OKLINK_BEGIN
//1 COMMAND
//2 PAYLOAD LENGTH
//3 Payload...

#define OKLINK_BEGIN 0xFE
#define OKLINK_N_HEADER 3
#define OKLINK_PWM 0
#define OKLINK_PIN_OUTPUT 1
#define OKLINK_STATUS 2
#define OKLINK_POS 3
#define OKLINK_BB 4

struct OKLINK_CMD
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

typedef void (*CallbackCMD)(uint8_t* pCMD, void* pInst);

namespace kai
{

class _OKlink: public _ThreadBase
{
public:
	_OKlink();
	~_OKlink();

	virtual bool init(void* pKiss);
	virtual bool start(void);
	virtual int check(void);
	virtual bool draw(void);
	virtual bool console(int& iY);

	virtual bool readCMD(void);
	virtual void handleCMD(void);
	void setCallback(CallbackCMD cb, void* pInst);

	void setPWM(int nChan, uint16_t* pChan);
	void pinOut(uint8_t iPin, uint8_t state);
	void sendBB(uint32_t id, uint16_t iClass, vFloat4& bb);

private:
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_OKlink *) This)->update();
		return NULL;
	}

public:
	_IOBase*	m_pIO;
	uint8_t*	m_pBuf;
	int			m_nBuf;
	OKLINK_CMD	m_recvMsg;
	uint64_t	m_nCMDrecv;

	CallbackCMD m_pfCallback;
	void*		m_pfInst;
};

}
#endif
