/*
 * _webSocket.h
 *
 *  Created on: August 8, 2016
 *      Author: yankai
 */

#ifndef OpenKAI_src_IO__webSocket_H_
#define OpenKAI_src_IO__webSocket_H_

#include "../Base/common.h"
#include "_IOBase.h"

namespace kai
{

struct WS_CLIENT
{
	uint32_t m_id;
};

class _webSocket: public _IOBase
{
public:
	_webSocket();
	virtual ~_webSocket();

	bool init(void* pKiss);
	bool link(void);
	bool start(void);
	bool open(void);
	void close(void);
	void reset(void);
	bool draw(void);

private:
	void readIO(void);
	void writeIO(void);

	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_webSocket*) This)->update();
		return NULL;
	}

public:
	string	m_fifo;
	int		m_fd;
};

}

#endif
