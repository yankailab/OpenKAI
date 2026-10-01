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

		virtual bool loadConfig(void) override;
		bool saveConfig(bool bExport) override;
		virtual void console(void *pConsole);

		bool open(void) override;
		void close(void) override;

	private:
		void readPackets(void) override;
		void writePackets(void) override;
		bool writePending(void);
		void closeConnection(uint64_t generation = 0);

	protected:
		BYTE_PACKET m_bpWrite;
		size_t m_iWrite = 0;
		uint64_t m_writeConnectionGeneration = 0;

		string m_strAddr = "";
		uint16_t m_port = 0;

		bool m_bClient = true;
		int m_socket = -1;
	};

}
#endif
