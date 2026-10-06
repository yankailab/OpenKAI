#include "_SocketCAN.h"
#include "../UI/_Console.h"
#include <cerrno>

namespace kai
{

	_SocketCAN::_SocketCAN()
	{
	}

	_SocketCAN::~_SocketCAN()
	{
		stop();
		close();
		DEL(m_pTr);
	}

	bool _SocketCAN::loadConfig(void)
	{
		stop();
		close();
		IF_F(!_ModuleBase::loadConfig());
		json &j = *m_pJ;

		if (!j.contains("threadR"))
		{
			j["threadR"] = {{"FPS", m_pT->getTargetFPS()}};
		}
		DEL(m_pTr);
		m_pTr = createThread(jK(j, "threadR"), "threadR");
		NULL_F(m_pTr);

		jKv(j, "ifName", m_ifName);
		jKv(j, "nErrReconnect", m_nErrReconnect);
		return true;
	}

	bool _SocketCAN::saveConfig(bool bExport)
	{
		IF_F(!_ModuleBase::saveConfig(false));
		IF_F(m_pTr && !m_pTr->saveConfig(false));

		json &j = *m_pJ;
		j["ifName"] = m_ifName;
		j["nErrReconnect"] = m_nErrReconnect;

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool _SocketCAN::link(InstanceMgr *pM)
	{
		IF_F(!_ModuleBase::link(pM));
		const json &j = *m_pJ;
		string n;

		jKv(j, "CANframeStreamIn", n);
		m_pCANframeIn = dynamic_cast<CANframeStream *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
		IF_Le_F(!n.empty() && !m_pCANframeIn, "CANframeStreamIn not found: " + n);

		n.clear();
		jKv(j, "CANframeStreamOut", n);
		m_pCANframeOut = dynamic_cast<CANframeStream *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
		IF_Le_F(!n.empty() && !m_pCANframeOut, "CANframeStreamOut not found: " + n);
		IF_Le_F(!m_pCANframeIn && !m_pCANframeOut, "CANframeStreamIn or CANframeStreamOut is required");

		m_tLastCANframeIn = 0;
		m_vFrameIn.clear();
		m_iFrameIn = 0;
		return true;
	}

	bool _SocketCAN::start(void)
	{
		IF_F(!m_pT || !m_pTr || !bStopped());
		if (!m_pT->startThread(getUpdateW, this) ||
			!m_pTr->startThread(getUpdateR, this))
		{
			stop();
			return false;
		}
		return true;
	}

	bool _SocketCAN::check(void)
	{
		IF_F(!m_pTr || !bOpen());
		IF_F(!m_pCANframeIn && !m_pCANframeOut);
		return _ModuleBase::check();
	}

	void _SocketCAN::pause(void)
	{
		if (m_pT)
		{
			m_pT->pause();
		}
		if (m_pTr)
		{
			m_pTr->pause();
		}
	}

	void _SocketCAN::resume(void)
	{
		if (m_pT)
		{
			m_pT->run();
		}
		if (m_pTr)
		{
			m_pTr->run();
		}
	}

	void _SocketCAN::stop(void)
	{
		// Stop both workers before joining, even when no socket is open.
		if (m_pT)
		{
			m_pT->stop();
		}
		if (m_pTr)
		{
			m_pTr->stop();
		}
		if (m_pT)
		{
			m_pT->join();
		}
		if (m_pTr)
		{
			m_pTr->join();
		}
	}

	bool _SocketCAN::bRun(void)
	{
		return (m_pT && m_pT->bRun()) || (m_pTr && m_pTr->bRun());
	}

	bool _SocketCAN::bRunning(void)
	{
		return (m_pT && m_pT->bRunning()) || (m_pTr && m_pTr->bRunning());
	}

	bool _SocketCAN::bStopped(void)
	{
		return (!m_pT || m_pT->bStopped()) && (!m_pTr || m_pTr->bStopped());
	}

	_Thread *_SocketCAN::getThread(const string &name)
	{
		if (m_pTr && m_pTr->getName() == name)
		{
			return m_pTr;
		}
		return _ModuleBase::getThread(name);
	}

	bool _SocketCAN::open(void)
	{
		std::unique_lock lock(m_connectionMutex);
		IF__(m_bOpened.load(), true);

		int fd = socket(PF_CAN, SOCK_RAW | SOCK_NONBLOCK | SOCK_CLOEXEC, CAN_RAW);
		if (fd < 0)
		{
			LOG_E("socket(PF_CAN)");
			return false;
		}

		struct ifreq ifr{};
		strncpy(ifr.ifr_name, m_ifName.c_str(), IFNAMSIZ - 1);
		if (ioctl(fd, SIOCGIFINDEX, &ifr) < 0)
		{
			LOG_E("ioctl(SIOCGIFINDEX)");
			::close(fd);
			return false;
		}

		struct sockaddr_can addr{};
		addr.can_family = AF_CAN;
		addr.can_ifindex = ifr.ifr_ifindex;
		if (bind(fd, reinterpret_cast<struct sockaddr *>(&addr), sizeof(addr)) < 0)
		{
			LOG_E("bind(AF_CAN)");
			::close(fd);
			return false;
		}

		m_socket = fd;
		m_iErr = 0;
		m_bOpened = true;
		return true;
	}

	bool _SocketCAN::bOpen(void)
	{
		return m_bOpened.load();
	}

	void _SocketCAN::close(void)
	{
		std::unique_lock lock(m_connectionMutex);
		m_bOpened = false;
		if (m_socket >= 0)
		{
			::close(m_socket);
			m_socket = -1;
		}
		m_iErr = 0;
	}

	void _SocketCAN::updateW(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPS();
			if (!m_pT->bRun())
			{
				break;
			}

			// The sender owns reconnects, including receive-only configurations.
			if (m_iErr.load() > m_nErrReconnect)
			{
				close();
			}
			if (!bOpen() && !open())
			{
				m_pT->sleepT(NSEC_SEC);
				continue;
			}
			sendFrame();
		}
	}

	void _SocketCAN::updateR(void)
	{
		while (m_pTr->bRun())
		{
			m_pTr->autoFPS();
			if (!m_pTr->bRun())
			{
				break;
			}
			if (readFrame())
			{
				m_pTr->skipSleep();
			}
		}
	}

	bool _SocketCAN::sendFrame(void)
	{
		IF_F(!m_pCANframeIn || !bOpen());
		if (m_iFrameIn == m_vFrameIn.size())
		{
			m_pCANframeIn->get(m_vFrameIn, m_tLastCANframeIn);
			m_iFrameIn = 0;
		}

		bool bSent = false;
		while (m_iFrameIn < m_vFrameIn.size())
		{
			const CAN_FRAME &f = m_vFrameIn[m_iFrameIn];
			const uint32_t idMask = f.m_bExtended ? CAN_EFF_MASK : CAN_SFF_MASK;
			if (f.m_nData > CAN_MAX_DLEN || f.m_ID > idMask)
			{
				LOG_E("Invalid classical CAN frame");
				m_tLastCANframeIn = std::max(m_tLastCANframeIn, f.m_tStamp);
				++m_iFrameIn;
				continue;
			}

			can_frame canF{};
			canF.can_id = f.m_ID;
			if (f.m_bExtended)
			{
				canF.can_id |= CAN_EFF_FLAG;
			}
			if (f.m_bRTR)
			{
				canF.can_id |= CAN_RTR_FLAG;
			}
			canF.len = f.m_nData;
			memcpy(canF.data, f.m_pData, f.m_nData);

			{
				std::shared_lock lock(m_connectionMutex);
				IF_F(!m_bOpened.load() || m_socket < 0);
				const ssize_t nW = ::write(m_socket, &canF, sizeof(canF));
				if (nW != sizeof(canF))
				{
					if (nW >= 0 || (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR))
					{
						LOG_E("write(can_frame)");
						++m_iErr;
					}
					// Retain the failed frame even if the input ring subsequently wraps.
					return false;
				}
			}

			m_tLastCANframeIn = std::max(m_tLastCANframeIn, f.m_tStamp);
			++m_iFrameIn;
			bSent = true;
		}
		return bSent;
	}

	bool _SocketCAN::readFrame(void)
	{
		NULL_F(m_pCANframeOut);
		can_frame canF{};
		{
			std::shared_lock lock(m_connectionMutex);
			IF_F(!m_bOpened.load() || m_socket < 0);
			const ssize_t nR = ::read(m_socket, &canF, sizeof(canF));
			if (nR != sizeof(canF))
			{
				if (nR >= 0 || (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR))
				{
					LOG_E("read(can_frame)");
					++m_iErr;
				}
				return false;
			}
		}

		IF_F(canF.len > CAN_MAX_DLEN || (canF.can_id & CAN_ERR_FLAG));
		CAN_FRAME f;
		f.m_bExtended = (canF.can_id & CAN_EFF_FLAG) != 0;
		f.m_bRTR = (canF.can_id & CAN_RTR_FLAG) != 0;
		f.m_ID = canF.can_id & (f.m_bExtended ? CAN_EFF_MASK : CAN_SFF_MASK);
		f.m_nData = canF.len;
		memcpy(f.m_pData, canF.data, canF.len);
		f.m_tStamp = std::max(getTns(), m_tLastCANframeOut + 1);
		m_tLastCANframeOut = f.m_tStamp;
		m_pCANframeOut->add({f}, f.m_tStamp);
		++m_nFrameRecv;
		return true;
	}

	void _SocketCAN::console(void *pConsole)
	{
		NULL_(pConsole);
		_ModuleBase::console(pConsole);
		if (m_pTr)
		{
			m_pTr->console(pConsole);
		}

		_Console *pC = static_cast<_Console *>(pConsole);
		pC->addMsg("bOpen = " + i2str(m_bOpened.load()), 1);
		pC->addMsg("nFrameRecv = " + i2str(m_nFrameRecv.load()), 1);
	}

}
