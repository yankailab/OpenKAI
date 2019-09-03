#ifndef OpenKAI_src_Protocol__OKlink_H_
#define OpenKAI_src_Protocol__OKlink_H_

#include "../Base/common.h"
#include "../Base/_ThreadBase.h"
#include "../IO/_IOBase.h"

#define OKLINK_N_BUF 256
#define OKLINK_BEGIN 0xFE
#define OKLINK_N_HEADER 3
#define OKLINK_PWM 0
#define OKLINK_PIN_OUTPUT 1
#define OKLINK_STATUS 2
#define OKLINK_POS 3

struct OKLINK_CMD
{
	int m_cmd;
	int m_nPayload;
	int m_iByte;
	char m_pBuf[OKLINK_N_BUF];

	void init(void)
	{
		m_cmd = 0;
		m_iByte = 0;
		m_nPayload = 0;
	}
};

namespace kai
{

class _OKlink: public _ThreadBase
{
public:
	_OKlink();
	~_OKlink();

	virtual bool init(void* pKiss);
	virtual bool start(void);
	virtual bool draw(void);
	virtual bool console(int& iY);

	virtual bool readCMD(void);
	virtual void handleCMD(void);

	void setPWM(int nChan, uint16_t* pChan);
	void pinOut(uint8_t iPin, uint8_t state);

private:
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_OKlink *) This)->update();
		return NULL;
	}

public:
	_IOBase*	m_pIO;
	uint8_t		m_pBuf[OKLINK_N_BUF];
	OKLINK_CMD	m_recvMsg;
	uint64_t	m_nCMDrecv;

};

}
#endif
