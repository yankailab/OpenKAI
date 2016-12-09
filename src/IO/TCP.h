#ifndef AI_IO_TCP_H_
#define AI_IO_TCP_H_

#include "../Base/common.h"
#include "../Network/_server.h"
#include "IO.h"

using namespace std;

namespace kai
{

class TCP : public IO
{
public:
	TCP(void);
	~TCP(void);

	//common
	bool init(void* pKiss);
	bool open(void);
	void close(void);

	int  read(uint8_t* pBuf, int nByte);

	bool write(uint8_t* pBuf, int nByte);
	bool writeLine(uint8_t* pBuf, int nByte);

	bool draw(Frame* pFrame, vInt4* pTextPos);

private:
	_server* m_pServer;
	_socket* m_pSocket;

};

}

#endif
