/*
 * _TCPclient.h
 *
 *  Created on: August 8, 2016
 *      Author: yankai
 */

#ifndef OpenKAI_src_IO__TCPclient_H_
#define OpenKAI_src_IO__TCPclient_H_

#include "_IObase.h"

namespace kai
{

	class _TCPclient : public _IObase
	{
	public:
		_TCPclient();
		virtual ~_TCPclient();

		int init(void *pKiss);
		int start(void);
		bool open(void);
		void close(void);
		void console(void *pConsole);
		bool bComplete(void);

	public:
		void updateW(void);
		static void *getUpdateW(void *This)
		{
			((_TCPclient *)This)->updateW();
			return NULL;
		}

		void updateR(void);
		static void *getUpdateR(void *This)
		{
			((_TCPclient *)This)->updateR();
			return NULL;
		}

	public:
		struct sockaddr_in m_serverAddr;
		string m_strAddr;
		uint16_t m_port;

		bool m_bClient;
		int m_socket;
		bool m_bComplete;
	};

}
#endif
