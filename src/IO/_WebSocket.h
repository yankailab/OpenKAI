/*
 * _webSocket.h
 *
 *  Created on: August 8, 2016
 *      Author: yankai
 */

#ifndef OpenKAI_src_IO__WebSocket_H_
#define OpenKAI_src_IO__WebSocket_H_

#include <wsserver/ws.h>
#include "_IObase.h"

#define WS_N_BUF 512

namespace kai
{
	class _WebSocket : public _IObase
	{
	public:
		_WebSocket();
		virtual ~_WebSocket();

		int init(void *pKiss);
		int start(void);
		void console(void *pConsole);

		//_IObase overwrite
		int read(uint8_t *pBuf, int nB);

	private:
		void updateW(void);
		static void *getUpdateW(void *This)
		{
			((_WebSocket *)This)->updateW();
			return NULL;
		}

		void updateR(void);
		static void *getUpdateR(void *This)
		{
			((_WebSocket *)This)->updateR();
			return NULL;
		}

	protected:
		pthread_mutex_t m_mutexW;

		_Thread *m_pTr;
	};

}
#endif
