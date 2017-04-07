#ifndef AI_IO_SERIALPORT_H_
#define AI_IO_SERIALPORT_H_

#include "../Base/common.h"
#include "IO.h"

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

class SerialPort: public IO
{
public:
	SerialPort();
	~SerialPort();

	bool init(void* pKiss);
	bool open(void);
	void close(void);

	int read(uint8_t* pBuf, int nByte);
	bool write(uint8_t* pBuf, int nByte);
	bool writeLine(uint8_t* pBuf, int nByte);

private:
	bool setup(void);

	//Connection status
	int m_fd;
	string m_name;
	int m_baud;
	int m_dataBits;
	int m_stopBits;
	bool m_parity;
	bool m_hardwareControl;

	pthread_mutex_t m_mutexWrite;
	pthread_mutex_t m_mutexRead;

};

}

#endif
