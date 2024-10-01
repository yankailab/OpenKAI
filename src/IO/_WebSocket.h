/*
 * _webSocket.h
 *
 *  Created on: August 8, 2016
 *      Author: yankai
 */

#ifndef OpenKAI_src_IO__WebSocket_H_
#define OpenKAI_src_IO__WebSocket_H_

#include "_IObase.h"

#define WS_N_HEADER 12
#define WS_N_BUF 1024
#define WS_N_MSG 10240

#define WS_MODE_TXT 0x1
#define WS_MODE_BIN 0x2

namespace kai
{

	struct WS_CLIENT
	{
		uint32_t m_id;
		IO_FIFO m_fifo;
		uint64_t m_tStamp;

		int init(uint32_t id, int nB)
		{
			m_id = id;
			m_tStamp = getApproxTbootUs();
			IF_F(!m_fifo.init(nB));
			reset();

			return true;
		}

		void reset(void)
		{
			m_fifo.clear();
		}
	};

	class _WebSocket : public _IObase
	{
	public:
		_WebSocket();
		virtual ~_WebSocket();

		int init(void *pKiss);
		int start(void);
		bool open(void);
		void close(void);
		void console(void *pConsole);

		//_IObase overwrite
		bool write(uint8_t *pBuf, int nB);
		int read(uint8_t *pBuf, int nB);

		// _WebSocket impl
		bool write(uint8_t *pBuf, int nB, uint32_t mode = WS_MODE_BIN);
		bool writeTo(uint32_t id, uint8_t *pBuf, int nB, uint32_t mode = WS_MODE_BIN);
		int readFrom(uint32_t id, uint8_t *pBuf, int nB);

		int nClient(void);

	private:
		void resetDecodeMsg(void);
		void decodeMsg(void);
		WS_CLIENT *findClientById(uint32_t id);

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
		string m_fifoIn;
		string m_fifoOut;
		int m_fdW;
		int m_fdR;
		pthread_mutex_t m_mutexW;

		vector<WS_CLIENT> m_vClient;

		int m_iMsg;
		int m_nMsg;
		int m_nB;
		int m_iB;
		WS_CLIENT *m_pC;
	};

}
#endif
