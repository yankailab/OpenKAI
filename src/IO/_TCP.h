#ifndef AI_IO_TCP_H_
#define AI_IO_TCP_H_

#include "../Base/common.h"
#include "../IO/_TCPserver.h"
#include "_IOBase.h"

namespace kai
{

class _TCP : public _IOBase
{
public:
	_TCP(void);
	~_TCP(void);

	//common
	bool init(void* pKiss);
	bool open(void);
	void close(void);

	int  read(uint8_t* pBuf, int nByte);

	bool write(uint8_t* pBuf, int nByte);
	bool writeLine(uint8_t* pBuf, int nByte);

	bool draw(void);

private:
	_TCPserver* m_pServer;
	_TCPsocket* m_pSocket;

};

}

#endif
