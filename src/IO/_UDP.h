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

		virtual bool loadConfig(void) override;
		bool saveConfig(bool bExport) override;
		virtual void console(void *pConsole);

		bool open(void);
		void close(void);

	protected:
		void readPackets(void) override;
		void writePackets(void) override;

	private:
		void closeConnection(uint64_t generation = 0);

	protected:
		string m_addrRemote = "";
		uint16_t m_portRemote = 0;
		uint16_t m_portLocal = 0;
		bool m_bW2R = true;	// write back to the client recevied from
		int m_bWbroadcast = 0;

		std::mutex m_remoteMutex;
		sockaddr_in m_sAddrLocal{};
		sockaddr_in m_sAddrRemote{};
		int m_socket = -1;
	};

}
#endif
