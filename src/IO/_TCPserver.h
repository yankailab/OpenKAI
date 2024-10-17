/*
 * _TCPserver.h
 *
 *  Created on: August 8, 2016
 *      Author: yankai
 */

#ifndef OpenKAI_src_IO__TCPserver_H_
#define OpenKAI_src_IO__TCPserver_H_

#include "../Base/_ModuleBase.h"
#include "../Module/Kiss.h"
#include "../IO/_IObase.h"
#include "_TCPclient.h"

#define N_LISTEN 1
#define N_SOCKET 1

namespace kai
{

	class _TCPserver : public _ModuleBase
	{
	public:
		_TCPserver();
		virtual ~_TCPserver();

		int init(void *pKiss);
		int start(void);
		void console(void *pConsole);
		_TCPclient *getFirstSocket(void);

		bool setup(void);
		void cleanupClient(void);
		bool handler(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_TCPserver *)This)->update();
			return NULL;
		}

	public:
		uint16_t m_listenPort;
		int m_nListen;

		int m_socket;
		struct sockaddr_in m_serverAddr;
		list<_TCPclient *> m_lClient;
		unsigned int m_nSocket;
	};

}
#endif
