/*
 * _UDP.h
 *
 *  Created on: June 16, 2016
 *      Author: yankai
 */

#ifndef OpenKAI_src_IO__UDP_H_
#define OpenKAI_src_IO__UDP_H_

#include "_IObase.h"

#define N_UDP_BUF 512

namespace kai
{

	class _UDP : public _IObase
	{
	public:
		_UDP();
		virtual ~_UDP();

		virtual bool init(const json& j);
		virtual bool start(void);
		virtual void console(void *pConsole);

		bool open(void);
		void close(void);
		int read(uint8_t *pBuf, int nB);

	private:
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_UDP *)This)->update();
			return NULL;
		}

	protected:
		string m_addrRemote = "";
		uint16_t m_portRemote = 0;
		uint16_t m_portLocal = 0;
		bool m_bW2R = true;	// write back to the client recevied from
		int m_bWbroadcast = 0;

		sockaddr_in m_sAddrLocal;
		sockaddr_in m_sAddrRemote;
		int m_socket = -1;
	};

}
#endif
