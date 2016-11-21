#ifndef OPENKAI_SRC_INTERFACE_CANBUS_H_
#define OPENKAI_SRC_INTERFACE_CANBUS_H_

#include "../Base/common.h"
#include "../Base/_ThreadBase.h"
#include "../IO/SerialPort.h"
#include "../Stream/Frame.h"


//0 START MARK
//1 PAYLOAD LENGTH
//2 COMMAND
//3 Payload...

#define MAVLINK_BEGIN 0xFE
#define MAVLINK_HEADDER_LEN 3

#define CMD_CAN_SEND 0
#define CAN_BUF 256

namespace kai
{

struct MESSAGE
{
	int m_cmd;
	int m_iByte;
	int m_payloadLen;
	char m_pBuf[CAN_BUF];
};

class _Canbus: public _ThreadBase
{
public:
	_Canbus();
	~_Canbus();

	bool init(Kiss* pKiss);
	bool link(void);
	void close(void);
	bool start(void);
	bool draw(Frame* pFrame, vInt4* pTextPos);

	void send(unsigned long addr, unsigned char len, unsigned char* pData);
	bool recv();

	MESSAGE m_recvMsg;

private:
	string	m_sportName;
	SerialPort* m_pSerialPort;
	int m_baudRate;

	unsigned char m_pBuf[CAN_BUF];

	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_Canbus *) This)->update();
		return NULL;
	}

};

}

#endif
