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

namespace kai
{

	class _SerialPort : public _IObase
	{
	public:
		_SerialPort();
		~_SerialPort();

		virtual int init(void *pKiss);
		virtual int link(void);
		virtual bool open(void);
		virtual void close(void);
		virtual int start(void);
		virtual void console(void *pConsole);

	private:
		bool setup(void);

		void updateW(void);
		static void *getUpdateW(void *This)
		{
			((_SerialPort *)This)->updateW();
			return NULL;
		}

		void updateR(void);
		static void *getUpdateR(void *This)
		{
			((_SerialPort *)This)->updateR();
			return NULL;
		}

	private:
		int m_fd;
		string m_port;
		int m_baud;
		int m_dataBits;
		int m_stopBits;
		bool m_parity;
		bool m_hardwareControl;
	};

}
#endif
