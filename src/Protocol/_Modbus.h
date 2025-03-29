#ifndef OpenKAI_src_Protocol__Modbus_H_
#define OpenKAI_src_Protocol__Modbus_H_

#include "../Base/_ModuleBase.h"
#include "../Dependencies/libmodbus/modbus.h"

namespace kai
{

	class _Modbus : public _ModuleBase
	{
	public:
		_Modbus();
		~_Modbus();

		int init(void *pKiss);
		int start(void);
		bool open(void);
		bool bOpen(void);

		int rawRequest(uint8_t *pB, int nB);
		int readInputBits(int iSlave, int addr, int nB, uint8_t *pB);
		int readRegisters(int iSlave, int addr, int nRegister, uint16_t *pB);
		int writeBit(int iSlave, int addr, bool bStatus);
		int writeRegister(int iSlave, int addr, int v);
		int writeRegisters(int iSlave, int addr, int nRegister, uint16_t *pB);

	private:
		void update(void);
		static void *getUpdate(void *This)
		{
			((_Modbus *)This)->update();
			return NULL;
		}

	protected:
		modbus_t *m_pMb;
		string m_type; // "RTU", "TCP"

		// RTU
		string m_rtuPort;
		string m_rtuParity;
		int m_rtuBaud;
		// TCP
		string m_tcpAddr;
		int m_tcpPort;

		bool m_bOpen;
		uint64_t m_tIntervalUsec;
		uint32_t m_tOutSec;
		uint32_t m_tOutUSec;

		pthread_mutex_t m_mutex;
	};

}
#endif
