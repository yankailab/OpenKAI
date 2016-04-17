#pragma once

#include "../Base/common.h"
#include "../IO/SerialPort.h"

#define MAVLINK_BEGIN 0xFE
#define MAVLINK_HEADDER_LEN 3

struct MESSAGE
{
	int m_cmd;
	int m_iByte;
	int m_payloadLen;
	char m_pBuf[256];

};

#define TRD_INTERVAL_VI_USEC 10000

namespace kai
{

class _VehicleInterface: public _ThreadBase
{
public:
	_VehicleInterface();
	~_VehicleInterface();

	bool setup(JSON* pJson, string serialName);
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
		((_VehicleInterface *) This)->update();
		return NULL;
	}

};

}

