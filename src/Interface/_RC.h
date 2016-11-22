#ifndef OPENKAI_SRC_INTERFACE_RC_H_
#define OPENKAI_SRC_INTERFACE_RC_H_

#include "../Base/common.h"
#include "../IO/SerialPort.h"
#include "../Base/_ThreadBase.h"
#include "../Script/Kiss.h"

#define MAVLINK_BEGIN 0xFE
#define MAVLINK_HEADDER_LEN 3

struct MESSAGE
{
	int m_cmd;
	int m_iByte;
	int m_payloadLen;
	char m_pBuf[256];

};

namespace kai
{

class _RC: public _ThreadBase
{
public:
	_RC();
	~_RC();

	bool init(void* pKiss);
	bool link(void);
	bool open(void);
	void close(void);
	bool start(void);

	bool readMessages();
//	void handleMessage(mavlink_message_t message);

	void rc_overide(int numChannel, int* pChannels);
	void controlMode(int mode);

	MESSAGE m_recvMsg;

private:
	bool m_bConnected;

	string	m_sportName;
	SerialPort* m_pSerialPort;
	int m_baudRate;

	char m_pBuf[256];

	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_RC *) This)->update();
		return NULL;
	}

};

}

#endif
