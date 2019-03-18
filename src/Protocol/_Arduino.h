#ifndef OpenKAI_src_Protocol__Arduino_H_
#define OpenKAI_src_Protocol__Arduino_H_

#include "../Base/common.h"
#include "../Base/_ThreadBase.h"
#include "../IO/_IOBase.h"

#define ARDU_CMD_N_BUF 256
#define ARDU_CMD_BEGIN 0xFE
#define ARDU_CMD_N_HEADER 3
#define ARDU_CMD_PWM 0
#define ARDU_CMD_PIN_OUTPUT 1
#define ARDU_CMD_STATUS 2

struct ARDUINO_CMD
{
	int m_cmd;
	int m_nPayload;
	int m_iByte;
	char m_pBuf[ARDU_CMD_N_BUF];

	void init(void)
	{
		m_cmd = 0;
		m_iByte = 0;
		m_nPayload = 0;
	}
};

namespace kai
{

class _Arduino: public _ThreadBase
{
public:
	_Arduino();
	~_Arduino();

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
		((_Arduino *) This)->update();
		return NULL;
	}

public:

	_IOBase*	m_pIO;
	uint8_t		m_pBuf[ARDU_CMD_N_BUF];
	ARDUINO_CMD m_recvMsg;
	uint64_t	m_nCMDrecv;

};

}
#endif
