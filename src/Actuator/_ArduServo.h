#ifndef OpenKAI_src_Actuator__ArduServo_H_
#define OpenKAI_src_Actuator__ArduServo_H_

#include "_ActuatorBase.h"
#include "../IO/_IObase.h"

#define ARDUSV_N_BUF 256
#define ARDUSV_BEGIN 0xFE
#define ARDUSV_N_HEADER 3
#define ARDU_CMD_PWM 0
#define ARDU_CMD_PIN_OUTPUT 1
#define ARDU_CMD_STATUS 2

namespace kai
{
	struct ARDUSERVO_CMD
	{
		int m_cmd;
		int m_nPayload;
		int m_iB;
		char m_pB[ARDUSV_N_BUF];

		void init(void)
		{
			m_cmd = 0;
			m_iB = 0;
			m_nPayload = 0;
		}
	};

	class _ArduServo : public _ActuatorBase
	{
	public:
		_ArduServo();
		~_ArduServo();

		virtual int init(void *pKiss);
		virtual int link(void);
		virtual int start(void);
		virtual void console(void *pConsole);

		virtual bool readCMD(void);
		virtual void handleCMD(void);

	private:
		void sendCMD(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_ArduServo *)This)->update();
			return NULL;
		}

		void updateR(void);
		static void *getUpdateR(void *This)
		{
			((_ArduServo *)This)->updateR();
			return NULL;
		}

	public:
		_Thread *m_pTr;
		_IObase *m_pIO;
		uint8_t m_pB[ARDUSV_N_BUF];
		ARDUSERVO_CMD m_recvMsg;
		uint64_t m_nCMDrecv;
	};

}
#endif
