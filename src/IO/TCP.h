#ifndef AI_IO_TCP_H_
#define AI_IO_TCP_H_

#include "../Base/common.h"
#include "IO.h"
#include "../Network/_peer.h"

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

private:
	_peer* m_pPeer;

};

}

#endif
