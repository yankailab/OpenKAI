/*
 * _UDP.cpp
 *
 *  Created on: June 16, 2016
 *      Author: yankai
 */

#include "_UDP.h"

namespace kai
{

	_UDP::_UDP()
	{
	}

	_UDP::~_UDP()
	{
		stop();
		close();
	}

	bool _UDP::loadConfig(void)
	{
		IF_F(!this->_IObase::loadConfig());
		const json &j = *m_pJ;

		jKv(j, "addrRemote", m_addrRemote);
		jKv(j, "portRemote", m_portRemote);
		jKv(j, "portLocal", m_portLocal);
		jKv(j, "bW2R", m_bW2R);
		jKv(j, "bWbroadcast", m_bWbroadcast);

		return true;
	}

	bool _UDP::saveConfig(bool bExport)
	{
		IF_F(!_IObase::saveConfig(false));

		json &j = *m_pJ;
		j["addrRemote"] = m_addrRemote;
		j["portRemote"] = m_portRemote;
		j["portLocal"] = m_portLocal;
		j["bW2R"] = m_bW2R;
		j["bWbroadcast"] = m_bWbroadcast;

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool _UDP::open(void)
	{
		std::unique_lock lock(m_connectionMutex);
		if (bOpen())
		{
			return true;
		}

		const int fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		IF_F(fd < 0);

		m_sAddrLocal = {};
		m_sAddrLocal.sin_family = AF_INET;
		m_sAddrLocal.sin_addr.s_addr = htonl(INADDR_ANY);
		m_sAddrLocal.sin_port = htons(m_portLocal);
		if (m_portLocal != 0 &&
			bind(fd, reinterpret_cast<sockaddr *>(&m_sAddrLocal), sizeof(m_sAddrLocal)) < 0)
		{
			::close(fd);
			return false;
		}

		if (m_bWbroadcast)
		{
			if (setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &m_bWbroadcast, sizeof(m_bWbroadcast)) < 0)
			{
				::close(fd);
				return false;
			}
			m_addrRemote = "255.255.255.255";
		}

		m_sAddrRemote = {};
		m_sAddrRemote.sin_family = AF_INET;
		m_sAddrRemote.sin_addr.s_addr = m_addrRemote.empty() ? htonl(INADDR_ANY) : inet_addr(m_addrRemote.c_str());
		m_sAddrRemote.sin_port = htons(m_portRemote);
		m_socket = fd;
		++m_connectionGeneration;
		m_ioStatus = io_opened;
		return true;
	}

	void _UDP::close(void)
	{
		closeConnection();
	}

	void _UDP::closeConnection(uint64_t generation)
	{
		std::unique_lock lock(m_connectionMutex);
		if (generation != 0 && generation != m_connectionGeneration.load())
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

	void _UDP::readPackets(void)
	{
		NULL_(m_pBpStreamOut);

		// A datagram remains one packet, including payloads larger than 512 bytes.
		uint8_t pB[65536];
		while (!m_pTr || m_pTr->bRun())
		{
			sockaddr_in sender{};
			socklen_t nSaddr = sizeof(sender);
			ssize_t nR;
			int error;
			uint64_t generation;
			{
				std::shared_lock lock(m_connectionMutex);
				if (!bOpen())
				{
					break;
				}
				generation = m_connectionGeneration.load();
				nR = ::recvfrom(m_socket, pB, sizeof(pB), MSG_DONTWAIT,
								reinterpret_cast<sockaddr *>(&sender), &nSaddr);
				error = errno;
				if (nR >= 0 && m_bW2R)
				{
					std::lock_guard<std::mutex> remoteLock(m_remoteMutex);
					m_sAddrRemote = sender;
				}
			}

			if (nR >= 0)
			{
				m_pBpStreamOut->addPacket(vector<uint8_t>(pB, pB + nR));
				LOG_I("Received " + i2str(nR) + " bytes from ip:" + string(inet_ntoa(sender.sin_addr)) + ", port:" + i2str(ntohs(sender.sin_port)));
				continue;
			}
			if (error == EINTR)
			{
				continue;
			}
			if (error != EAGAIN && error != EWOULDBLOCK)
			{
				LOG_E("recvfrom error: " + i2str(error));
				closeConnection(generation);
			}
			break;
		}
	}

	void _UDP::writePackets(void)
	{
		NULL_(m_pBpStreamIn);

		vector<BYTE_PACKET> vBp;
		m_pBpStreamIn->getPackets(vBp, m_tLastBpStreamIn);
		for (const BYTE_PACKET &bp : vBp)
		{
			if (m_pT && !m_pT->bRun())
			{
				break;
			}
			ssize_t nSend;
			int error;
			sockaddr_in remote;
			{
				std::shared_lock lock(m_connectionMutex);
				if (!bOpen())
				{
					break;
				}
				{
					std::lock_guard<std::mutex> remoteLock(m_remoteMutex);
					remote = m_sAddrRemote;
				}
				if (remote.sin_addr.s_addr == htonl(INADDR_ANY))
				{
					break;
				}
				do
				{
					nSend = ::sendto(m_socket, bp.m_vB.data(), bp.m_vB.size(), MSG_DONTWAIT,
									 reinterpret_cast<sockaddr *>(&remote), sizeof(remote));
				}
				while (nSend < 0 && errno == EINTR);
				error = errno;
			}

			if (nSend < 0)
			{
				if (error != EAGAIN && error != EWOULDBLOCK)
				{
					LOG_E("sendto error: " + i2str(error));
				}
				break;
			}

			m_tLastBpStreamIn = bp.m_tStamp;
			LOG_I("send: " + i2str(nSend) + " bytes to " + string(inet_ntoa(remote.sin_addr)) + ", port:" + i2str(ntohs(remote.sin_port)));
		}
	}

	void _UDP::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_IObase::console(pConsole);

		_Console *pC = (_Console *)pConsole;
		pC->addMsg("PortLocal:" + i2str(m_portLocal));
		pC->addMsg("PortRemote:" + i2str(m_portRemote));
	}

}
