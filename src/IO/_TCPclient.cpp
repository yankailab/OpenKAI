/*
 * _TCPclient.cpp
 *
 *  Created on: August 8, 2016
 *      Author: yankai
 */

#include "_TCPclient.h"
#include <poll.h>

namespace kai
{

	_TCPclient::_TCPclient()
	{
		m_ioStatus = io_unknown;
	}

	_TCPclient::~_TCPclient()
	{
		stop();
		close();
	}

	bool _TCPclient::loadConfig(void)
	{
		IF_F(!this->_IObase::loadConfig());
		const json &j = *m_pJ;

		jKv(j, "addr", m_strAddr);
		jKv(j, "port", m_port);
		m_bClient = true;

		return true;
	}

	bool _TCPclient::saveConfig(bool bExport)
	{
		IF_F(!_IObase::saveConfig(false));

		json &j = *m_pJ;
		j["addr"] = m_strAddr;
		j["port"] = m_port;

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool _TCPclient::open(void)
	{
		std::unique_lock<std::shared_mutex> lock(m_connectionMutex);
		if (bOpen())
		{
			return true;
		}

		int socket = ::socket(AF_INET, SOCK_STREAM, 0);
		if (socket < 0)
		{
			return false;
		}
		if (::fcntl(socket, F_SETFL, O_NONBLOCK) < 0)
		{
			::close(socket);
			return false;
		}

		struct sockaddr_in server = {};
		server.sin_addr.s_addr = inet_addr(m_strAddr.c_str());
		server.sin_family = AF_INET;
		server.sin_port = htons(m_port);
		LOG_I("connecting");

		int ret = ::connect(socket, reinterpret_cast<struct sockaddr *>(&server), sizeof(server));
		if (ret < 0 && errno == EINPROGRESS)
		{
			// Bound connection attempts so stopping the writer can always finish.
			struct pollfd connection = {socket, POLLOUT, 0};
			ret = ::poll(&connection, 1, 1000);
			if (ret > 0)
			{
				int error = 0;
				socklen_t nError = sizeof(error);
				ret = ::getsockopt(socket, SOL_SOCKET, SO_ERROR, &error, &nError);
				if (error != 0)
				{
					ret = -1;
				}
			}
			else
			{
				ret = -1;
			}
		}
		if (ret < 0)
		{
			::close(socket);
			LOG_E("connect failed");
			return false;
		}

		m_socket = socket;
		++m_connectionGeneration;
		m_ioStatus = io_opened;
		LOG_I("connected");
		return true;
	}

	void _TCPclient::close(void)
	{
		closeConnection();
	}

	void _TCPclient::closeConnection(uint64_t generation)
	{
		std::unique_lock<std::shared_mutex> lock(m_connectionMutex);
		if (generation != 0 && generation != m_connectionGeneration)
		{
			return;
		}
		if (m_socket >= 0)
		{
			::close(m_socket);
			m_socket = -1;
		}
		_IObase::close();
	}

	void _TCPclient::readPackets(void)
	{
		if (!m_pBpStreamOut)
		{
			return;
		}

		uint8_t pB[N_TCP_BUF];
		while (bOpen())
		{
			if (m_pTr && !m_pTr->bRun())
			{
				return;
			}

			ssize_t nR;
			int error;
			uint64_t generation;
			{
				std::shared_lock<std::shared_mutex> lock(m_connectionMutex);
				if (!bOpen() || m_socket < 0)
				{
					return;
				}
				generation = m_connectionGeneration;
				nR = ::recv(m_socket, pB, sizeof(pB), MSG_DONTWAIT);
				error = errno;
			}
			if (nR > 0)
			{
				m_pBpStreamOut->addPacket(vector<uint8_t>(pB, pB + nR));
				continue;
			}
			if (nR < 0 && error == EINTR)
			{
				continue;
			}
			if (nR < 0 && (error == EAGAIN || error == EWOULDBLOCK))
			{
				return;
			}

			LOG_E("recv closed or failed: " + i2str(nR < 0 ? error : 0));
			closeConnection(generation);
			return;
		}
	}

	void _TCPclient::writePackets(void)
	{
		uint64_t generation = m_connectionGeneration;
		if (m_writeConnectionGeneration != generation)
		{
			// A reconnect restarts the pending packet; only the writer owns its offset.
			m_iWrite = 0;
			m_writeConnectionGeneration = generation;
		}
		if (!writePending() || !m_pBpStreamIn)
		{
			return;
		}

		vector<BYTE_PACKET> vBp;
		m_pBpStreamIn->getPackets(vBp, m_tLastBpStreamIn);
		for (const BYTE_PACKET &bp : vBp)
		{
			m_bpWrite = bp;
			if (!writePending())
			{
				break;
			}
		}
	}

	bool _TCPclient::writePending(void)
	{
		while (m_iWrite < m_bpWrite.m_vB.size())
		{
			if (m_pT && !m_pT->bRun())
			{
				return false;
			}

			ssize_t nW;
			int error;
			{
				std::shared_lock<std::shared_mutex> lock(m_connectionMutex);
				if (!bOpen() || m_socket < 0 || m_writeConnectionGeneration != m_connectionGeneration)
				{
					return false;
				}
				nW = ::send(m_socket, m_bpWrite.m_vB.data() + m_iWrite,
							 m_bpWrite.m_vB.size() - m_iWrite, MSG_DONTWAIT | MSG_NOSIGNAL);
				error = errno;
			}
			if (nW < 0 && error == EINTR)
			{
				continue;
			}
			if (nW < 0 && error != EAGAIN && error != EWOULDBLOCK)
			{
				LOG_E("send error: " + i2str(error));
				closeConnection(m_writeConnectionGeneration);
				return false;
			}
			if (nW <= 0)
			{
				return false;
			}

			m_iWrite += static_cast<size_t>(nW);
			LOG_I("send: " + i2str(nW) + " bytes");
		}

		if (m_bpWrite.m_tStamp > m_tLastBpStreamIn)
		{
			m_tLastBpStreamIn = m_bpWrite.m_tStamp;
		}
		m_bpWrite.clear();
		m_iWrite = 0;
		return true;
	}

	void _TCPclient::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_IObase::console(pConsole);

		string msg = "Peer IP: " + m_strAddr + ":" + i2str(m_port) + ((m_bClient) ? "; Client" : "; Server");
		((_Console *)pConsole)->addMsg(msg);
	}

}
