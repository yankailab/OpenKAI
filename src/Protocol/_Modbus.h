#ifndef OpenKAI_src_Protocol__Modbus_H_
#define OpenKAI_src_Protocol__Modbus_H_

#include "../Base/_ModuleBase.h"
#include <modbus.h>
#include <mutex>

namespace kai
{

	class _Modbus : public _ModuleBase
	{
	public:
		_Modbus();
		~_Modbus();

		virtual bool loadConfig(void) override;
		bool saveConfig(bool bExport) override;
		virtual bool start(void);
		void stop(void) override;
		virtual bool check(void);
		// open() connects with the current settings and enables automatic retries.
		// close() waits for in-flight I/O, releases the context, and disables retries
		// until open() is called. Set connection parameters between close/open.
		bool open(void);
		bool bOpen(void);
		void close(void);

		// Thread-safe settings; transport/debug/timeout changes apply on the next
		// connection. Error threshold and write interval apply immediately.
		// open()/loadConfig() validate the settings. Serialize configuration
		// sequences against other writers; each individual call is synchronized.
		void setType(const string &value);
		string getType(void) const;
		void setRtuPort(const string &value);
		string getRtuPort(void) const;
		void setRtuParity(const string &value);
		string getRtuParity(void) const;
		void setRtuBaud(int value);
		int getRtuBaud(void) const;
		void setRtuDataBits(int value);
		int getRtuDataBits(void) const;
		void setRtuStopBits(int value);
		int getRtuStopBits(void) const;
		void setTcpAddr(const string &value);
		string getTcpAddr(void) const;
		void setTcpPort(int value);
		int getTcpPort(void) const;
		void setModbusDebug(bool value);
		bool getModbusDebug(void) const;
		void setErrReconnect(int value);
		int getErrReconnect(void) const;
		void setIntervalUsec(uint64_t value);
		uint64_t getIntervalUsec(void) const;
		void setResponseTimeout(uint32_t sec, uint32_t usec);
		void getResponseTimeout(uint32_t &sec, uint32_t &usec) const;

		int rawRequest(uint8_t *pB, int nB);
		int readInputBits(int iSlave, int addr, int nB, uint8_t *pB);
		int readRegisters(int iSlave, int addr, int nRegister, uint16_t *pB);
		int writeBit(int iSlave, int addr, bool bStatus);
		int writeRegister(int iSlave, int addr, int v);
		int writeRegisters(int iSlave, int addr, int nRegister, uint16_t *pB);
		int sendRawRequest(uint8_t* pB, int nB);
		int sendRawRequest(int iSlave, uint8_t* pB, int nB);

	private:
		// These helpers require m_mutex to be held by the caller.
		bool validConfigLocked(void);
		bool openLocked(void);
		void closeLocked(void);
		bool checkLocked(void);

		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_Modbus *)This)->update();
			return NULL;
		}

	protected:
		modbus_t *m_pMb = nullptr;
		string m_type = "RTU"; // "RTU", "TCP"
		bool m_bModbusDebug = false;
		int m_nErrReconnect = 0; // Reconnect when accumulated errors exceed this threshold.
		int m_iErr = 0;

		// RTU
		string m_rtuPort = "";
		string m_rtuParity = "E";
		int m_rtuBaud = 115200;
		int m_rtuDataBits = 8;
		int m_rtuStopBits = 1;
		// TCP
		string m_tcpAddr = "";
		int m_tcpPort = 0;

		bool m_bOpened = false;
		bool m_bConnectRequested = true; // Preserve automatic connection on start.
		uint64_t m_tIntervalUsec = 10000;
		uint32_t m_tOutSec = 1;
		uint32_t m_tOutUSec = 0;

		mutable std::mutex m_mutex;
	};

}
#endif
