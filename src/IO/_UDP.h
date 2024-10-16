/*
 * _UDP.h
 *
 *  Created on: June 16, 2016
 *      Author: yankai
 */

#ifndef OpenKAI_src_IO__UDP_H_
#define OpenKAI_src_IO__UDP_H_

#include "_IObase.h"

namespace kai
{

	class _UDP : public _IObase
	{
	public:
		_UDP();
		virtual ~_UDP();

		int init(void *pKiss);
		int start(void);
		void close(void);
		void console(void *pConsole);
		bool open(void);

	private:
		void updateW(void);
		static void *getUpdateW(void *This)
		{
			((_UDP *)This)->updateW();
			return NULL;
		}

		void updateR(void);
		static void *getUpdateR(void *This)
		{
			((_UDP *)This)->updateR();
			return NULL;
		}

	protected:
		string m_addr;
		uint16_t m_port;
		bool m_bServer;
		bool m_bW2R;	// write back to the client recevied from
		int m_bWbroadcast;

		sockaddr_in m_sAddrW;
		sockaddr_in m_sAddrR;
		unsigned int m_nSAddr;
		int m_socket;
		uint32_t m_addrAny;
	};

}
#endif
