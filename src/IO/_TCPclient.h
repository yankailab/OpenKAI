/*
 * _TCPclient.h
 *
 *  Created on: August 8, 2016
 *      Author: yankai
 */

#ifndef OpenKAI_src_IO__TCPclient_H_
#define OpenKAI_src_IO__TCPclient_H_

#include "_IObase.h"

#define N_TCP_BUF 512

namespace kai
{

	class _TCPclient : public _IObase
	{
	public:
		_TCPclient();
		virtual ~_TCPclient();

		virtual bool init(const json& j);
		virtual bool start(void);
		virtual void console(void *pConsole);

		bool open(void);
		void close(void);

		virtual int read(uint8_t *pBuf, int nB);

	private:
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_TCPclient *)This)->update();
			return NULL;
		}

	protected:
		struct sockaddr_in m_serverAddr;
		string m_strAddr;
		uint16_t m_port;

		bool m_bClient;
		int m_socket;
	};

}
#endif
