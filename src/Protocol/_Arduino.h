#ifndef OpenKAI_src_Protocol__Arduino_H_
#define OpenKAI_src_Protocol__Arduino_H_

#include "../Base/common.h"
#include "../Base/_ThreadBase.h"
#include "../IO/_IOBase.h"

#define CMD_BEGIN 0xFE
#define CMD_HEADDER_LEN 3
#define CMD_BUF_LEN 256
#define CMD_PWM 0
#define CMD_PIN_OUTPUT 1
#define CMD_STATUS 2

struct ARDUINO_MESSAGE
{
	int m_cmd;
	int m_iByte;
	int m_payloadLen;
	char m_pBuf[256];

	void init(void)
	{
		m_cmd = 0;
		m_iByte = 0;
		m_payloadLen = 0;
	}
};

namespace kai
{

class _Arduino: public _ThreadBase
{
public:
	_Arduino();
	~_Arduino();

	bool init(void* pKiss);
	bool start(void);
	bool draw(void);
	bool console(int& iY);

	void readMessages(void);
	void handleMessage(void);
	void setPWM(int nChan, int* pChan);
	void pinOut(int iPin, int state);

public:
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_Arduino *) This)->update();
		return NULL;
	}

	_IOBase* m_pIO;
	uint8_t m_pBuf[CMD_BUF_LEN];
	ARDUINO_MESSAGE m_recvMsg;

	uint32_t m_pState[8];

};

}
#endif
