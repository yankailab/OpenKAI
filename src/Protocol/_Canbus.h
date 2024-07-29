#ifndef OpenKAI_src_Protocol__Canbus_H_
#define OpenKAI_src_Protocol__Canbus_H_

#include "../Base/_ModuleBase.h"
#include "../IO/_SerialPort.h"

//0 START MARK
//1 PAYLOAD LENGTH
//2 COMMAND
//3 Payload...

#define MAVLINK_BEGIN 0xFE
#define MAVLINK_HEADDER_LEN 3

#define CAN_CMD_CAN_SEND 0
#define CAN_CMD_PIN_OUTPUT 2
#define CAN_BUF 256

#define N_CANDATA 256

namespace kai
{

	struct CAN_MESSAGE
	{
		int m_cmd;
		int m_iByte;
		int m_payloadLen;
		char m_pBuf[CAN_BUF];

		void init(void)
		{
			m_cmd = 0;
			m_iByte = 0;
			m_payloadLen = 0;
		}
	};

	struct CAN_DATA
	{
		uint32_t m_addr;
		uint8_t m_len;
		uint8_t m_pData[8];

		void init(void)
		{
			m_addr = 0;
			m_len = 0;
		}
	};

	class _Canbus : public _ModuleBase
	{
	public:
		_Canbus();
		~_Canbus();

		int init(void *pKiss);
		int start(void);
		void console(void *pConsole);

		void send(unsigned long addr, unsigned char len, unsigned char *pData);
		void pinOut(uint8_t pin, uint8_t output);
		bool recv();
		uint8_t *get(unsigned long addr);

	public:
		void recvMsg(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_Canbus *)This)->update();
			return NULL;
		}

		_SerialPort *m_pIO;

		CAN_MESSAGE m_recvMsg;
		CAN_DATA m_pCanData[N_CANDATA];
		int m_nCanData;
	};

}
#endif
