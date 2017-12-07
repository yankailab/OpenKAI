#ifndef OpenKAI_src_IO__SerialPort_H_
#define OpenKAI_src_IO__SerialPort_H_

#include "../Base/common.h"
#include "_IOBase.h"

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

class _SerialPort: public _IOBase
{
public:
	_SerialPort();
	~_SerialPort();

	bool init(void* pKiss);
	bool open(void);
	void close(void);
	bool start(void);
	void reset(void);

private:
	bool setup(void);
	void readIO(void);
	void writeIO(void);

	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_SerialPort*) This)->update();
		return NULL;
	}

private:
	int m_fd;
	string m_name;
	int m_baud;
	int m_dataBits;
	int m_stopBits;
	bool m_parity;
	bool m_hardwareControl;

};

}

#endif
