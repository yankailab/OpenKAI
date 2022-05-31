#ifndef OpenKAI_src_Protocol__Modbus_H_
#define OpenKAI_src_Protocol__Modbus_H_

#include "../Base/_ModuleBase.h"
#include "../Dependency/libmodbus/modbus.h"

namespace kai
{

	class _Modbus : public _ModuleBase
	{
	public:
		_Modbus();
		~_Modbus();

		bool init(void *pKiss);
		bool start(void);
		bool open(void);
		bool bOpen(void);

		int rawRequest(uint8_t *pB, int nB);
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

	public:
		modbus_t *m_pMb;
		string m_port;
		string m_parity;
		int m_baud;
		bool m_bOpen;

		uint32_t m_tOutSec;
		uint32_t m_tOutUSec;

		pthread_mutex_t m_mutex;
	};

}
#endif
