#ifndef OpenKAI_src_IO__SerialPort_H_
#define OpenKAI_src_IO__SerialPort_H_

#include "_IObase.h"

// The following two non-standard baudrates should have been defined by the system
// If not, just fallback to number
#ifndef B460800
#define B460800 460800
#endif

#ifndef B921600
#define B921600 921600
#endif

using namespace std;

#define N_SERIAL_BUF 512

namespace kai
{

	class _SerialPort : public _IObase
	{
	public:
		_SerialPort();
		~_SerialPort();

		virtual bool loadConfig(void) override;
		bool saveConfig(bool bExport) override;
		virtual bool link(InstanceMgr *pM) override;
		virtual void console(void *pConsole);

		bool open(void) override;
		void close(void) override;

	private:
		void readPackets(void) override;
		void writePackets(void) override;
		bool writePending(void);
		bool setup(void);
		void closeConnection(uint64_t generation = 0);

	protected:
		BYTE_PACKET m_bpWrite;
		size_t m_iWrite = 0;
		uint64_t m_writeConnectionGeneration = 0;

		int m_fd = -1;
		string m_port = "";
		int m_baud = 115200;
		int m_dataBits = 8;
		int m_stopBits = 1;
		bool m_parity = false;
		bool m_hardwareControl = false;
	};

}
#endif
