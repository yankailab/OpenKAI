#ifndef OpenKAI_src_Actuator__ArduServo_H_
#define OpenKAI_src_Actuator__ArduServo_H_

#include "_ActuatorBase.h"
#include "../IO/_IOBase.h"

#define ARDU_CMD_N_BUF 256
#define ARDU_CMD_BEGIN 0xFE
#define ARDU_CMD_N_HEADER 3
#define ARDU_CMD_PWM 0
#define ARDU_CMD_PIN_OUTPUT 1
#define ARDU_CMD_STATUS 2

struct ARDUSERVO_CMD
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

struct ARDUSERVO_CHAN
{
	uint16_t m_pwmL;
	uint16_t m_pwmH;
	float m_dir;

	void init(void)
	{
		m_pwmL = 1000;
		m_pwmH = 2000;
		m_dir = 1.0;
	}
};

namespace kai
{

class _ArduServo: public _ActuatorBase
{
public:
	_ArduServo();
	~_ArduServo();

	virtual bool init(void* pKiss);
	virtual bool start(void);
	virtual bool draw(void);
	virtual bool console(int& iY);

	virtual bool readCMD(void);
	virtual void handleCMD(void);

private:
	void updatePWM(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_ArduServo *) This)->update();
		return NULL;
	}

public:
	_IOBase*	m_pIO;
	uint8_t		m_pBuf[ARDU_CMD_N_BUF];
	ARDUSERVO_CMD m_recvMsg;
	uint64_t	m_nCMDrecv;

	vector<ARDUSERVO_CHAN> m_vServo;


};

}
#endif
