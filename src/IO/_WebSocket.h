/*
 * _webSocket.h
 *
 *  Created on: August 8, 2016
 *      Author: yankai
 */

#ifndef OpenKAI_src_IO__WebSocket_H_
#define OpenKAI_src_IO__WebSocket_H_

#include "../Base/common.h"
#include "_IOBase.h"

#define WS_N_HEADER 12
#define WS_N_BUF 1024
#define WS_N_MSG 10240

#define WS_MODE_TXT 0x1
#define WS_MODE_BIN 0x2

/*
namespace kai
{

struct WS_CLIENT
{
	uint32_t m_id;
	std::queue<IO_BUF> m_qR;
	uint64_t m_tStamp;

	void init(uint32_t id)
	{
		m_id = id;
		m_tStamp = getTimeUsec();
		reset();
	}

	void reset(void)
	{
		while (!m_qR.empty())m_qR.pop();
	}
};

class _WebSocket: public _IOBase
{
public:
	_WebSocket();
	virtual ~_WebSocket();

	bool init(void* pKiss);
	bool start(void);
	bool open(void);
	void close(void);
	bool draw(void);

	bool write(uint8_t* pBuf, int nB);
	bool write(uint8_t* pBuf, int nB, uint32_t mode);
	int  read(uint8_t* pBuf, int nB);

	bool writeTo(uint32_t id, uint8_t* pBuf, int nB, uint32_t mode);
	int  readFrom(uint32_t id, uint8_t* pBuf, int nB);

private:
	int	 readFrameBuf(uint8_t* pBuf, int nB);
	void resetDecodeMsg(void);
	void decodeMsg(void);
	WS_CLIENT* findClientById(uint32_t id);

	void updateW(void);
	static void* getUpdateThreadW(void* This)
	{
		((_WebSocket*) This)->updateW();
		return NULL;
	}

	void updateR(void);
	static void* getUpdateThreadR(void* This)
	{
		((_WebSocket*) This)->updateR();
		return NULL;
	}

public:
	string	m_fifoW;
	string	m_fifoR;
	int		m_fdW;
	int		m_fdR;
	uint32_t m_mode;

	vector<WS_CLIENT> m_vClient;
	pthread_mutex_t m_mutexCR;

	int m_iMsg;
	int m_nMsg;
	int m_nB;
	int m_iB;
	WS_CLIENT* m_pC;
	uint8_t m_pMB[WS_N_BUF];

};

}
*/
#endif
