#ifndef OpenKAI_src_Protocol__Modbus_H_
#define OpenKAI_src_Protocol__Modbus_H_

#include "../Base/common.h"
#include "../Base/_ThreadBase.h"
#include "../Dependency/libmodbus/modbus.h"

namespace kai
{

class _Modbus: public _ThreadBase
{
public:
	_Modbus();
	~_Modbus();

	bool init(void* pKiss);
	bool start(void);
	bool draw(void);

	bool open(void);
	bool bOpen(void);

	int rawRequest(uint8_t* pB, int nB);
	int writeRegisters(int iSlave, int addr, int nRegister, uint16_t* pB);
	int readRegisters(int iSlave, int addr, int nRegister, uint16_t* pB);

private:
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_Modbus*) This)->update();
		return NULL;
	}

public:
	modbus_t *m_pMb;
	string	m_port;
	string	m_parity;
	int		m_baud;
	bool	m_bOpen;

	pthread_mutex_t m_mutex;

};

}

#endif
