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
#define WS_N_MSG 1024

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
	bool link(void);
	bool start(void);
	bool open(void);
	void close(void);
	void reset(void);
	bool draw(void);

	bool writeTo(uint32_t id, uint8_t* pBuf, int nB);
	int  readFrom(uint32_t id, uint8_t* pBuf, int nB);

private:
	void readIO(void);
	void writeIO(void);
	void decodeMsg(void);
	WS_CLIENT* findClientById(uint32_t id);

	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_WebSocket*) This)->update();
		return NULL;
	}

public:
	string	m_fifo;
	int		m_fd;

	vector<WS_CLIENT> m_vClient;

	int	m_iMB;
	uint8_t m_pMB[WS_N_MSG];

};

}

#endif
