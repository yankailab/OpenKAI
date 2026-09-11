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

		virtual bool init(const json& j);
		virtual bool link(const json& j, ModuleMgr* pM);
		virtual bool start(void);
		virtual void console(void *pConsole);

		virtual bool open(void);
		virtual void close(void);
		virtual int read(uint8_t *pBuf, int nB);

	private:
		bool setup(void);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_SerialPort *)This)->update();
			return NULL;
		}

	protected:
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
