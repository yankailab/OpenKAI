#ifndef OpenKAI_src_Protocol__RC_H_
#define OpenKAI_src_Protocol__RC_H_

#include "../Base/common.h"
#include "../Base/_ThreadBase.h"
#include "../IO/_SerialPort.h"
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
	void close(void);
	bool start(void);

	bool readMessages();
	void rc_overide(int numChannel, int* pChannels);
	void controlMode(int mode);

	MESSAGE m_recvMsg;

private:
	_IOBase* m_pIO;
	uint8_t m_pBuf[256];

	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_RC *) This)->update();
		return NULL;
	}

};

}

#endif
