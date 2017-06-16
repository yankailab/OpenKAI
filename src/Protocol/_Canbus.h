#ifndef OPENKAI_SRC_INTERFACE_CANBUS_H_
#define OPENKAI_SRC_INTERFACE_CANBUS_H_

#include "../Base/common.h"
#include "../Base/_ThreadBase.h"
#include "../IO/_SerialPort.h"
#include "../Vision/Frame.h"


//0 START MARK
//1 PAYLOAD LENGTH
//2 COMMAND
//3 Payload...

#define MAVLINK_BEGIN 0xFE
#define MAVLINK_HEADDER_LEN 3

#define CMD_CAN_SEND 0
#define CMD_PIN_OUTPUT 2
#define CAN_BUF 256

#define N_CANDATA 256

namespace kai
{

struct MESSAGE
{
	int m_cmd;
	int m_iByte;
	int m_payloadLen;
	char m_pBuf[CAN_BUF];
};

struct CAN_DATA
{
	uint32_t		m_addr;
	uint8_t			m_len;
	uint8_t			m_pData[8];

	void init(void)
	{
		m_addr = 0;
		m_len = 0;
	}
};

class _Canbus: public _ThreadBase
{
public:
	_Canbus();
	~_Canbus();

	bool init(void* pKiss);
	bool link(void);
	void close(void);
	bool start(void);
	bool draw(void);

	void send(unsigned long addr, unsigned char len, unsigned char* pData);
	void pinOut(uint8_t pin, uint8_t output);
	bool recv();
	uint8_t* get(unsigned long addr);

public:
	void recvMsg(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_Canbus *) This)->update();
		return NULL;
	}

	_SerialPort* m_pSerialPort;

	MESSAGE m_recvMsg;
	CAN_DATA m_pCanData[N_CANDATA];
	int m_nCanData;

};

}

#endif
