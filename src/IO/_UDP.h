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

		int init(void *pKiss);
		int start(void);
		void console(void *pConsole);

		bool open(void);
		void close(void);
		int read(uint8_t *pBuf, int nB);

	private:
		void update(void);
		static void *getUpdate(void *This)
		{
			((_UDP *)This)->update();
			return NULL;
		}

	protected:
		string m_addr;
		uint16_t m_port;
		bool m_bServer;
		bool m_bW2R;	// write back to the client recevied from
		int m_bWbroadcast;

		sockaddr_in m_sAddrW;
//		sockaddr_in m_sAddrR;
		int m_socket;
	};

}
#endif
