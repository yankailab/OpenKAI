#include "_SocketCAN.h"

namespace kai
{

	_SocketCAN::_SocketCAN()
	{
		m_socket = -1;
	}

	_SocketCAN::~_SocketCAN()
	{
		close();
	}

	bool _SocketCAN::init(const json &j)
	{
		IF_F(!this->_CANbase::init(j));

		m_ifName = j.value("ifName", "can0");

		return true;
	}

	bool _SocketCAN::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_CANbase::link(j, pM));

		return true;
	}

	bool _SocketCAN::open(void)
	{
		m_socket = socket(PF_CAN, SOCK_RAW, CAN_RAW);
		if (m_socket < 0)
		{
			LOG_E("socket(PF_CAN)");
			return false;
		}

		struct ifreq ifr;
		memset(&ifr, 0, sizeof(ifr));
		strncpy(ifr.ifr_name, m_ifName.c_str(), IFNAMSIZ - 1);
		if (ioctl(m_socket, SIOCGIFINDEX, &ifr) < 0)
		{
			LOG_E("ioctl(SIOCGIFINDEX)");
			::close(m_socket);
			return false;
		}

		struct sockaddr_can addr;
		memset(&addr, 0, sizeof(addr));
		addr.can_family = AF_CAN;
		addr.can_ifindex = ifr.ifr_ifindex;

		if (bind(m_socket, (struct sockaddr *)&addr, sizeof(addr)) < 0)
		{
			LOG_E("bind(AF_CAN)");
			::close(m_socket);
			return false;
		}

		m_bOpen = true;
		return true;
	}

	void _SocketCAN::close(void)
	{
		m_bOpen = false;
		::close(m_socket);
		m_iErr = 0;
	}

	bool _SocketCAN::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	bool _SocketCAN::check(void)
	{
		IF_F(!m_bOpen);

		return this->_CANbase::check();
	}

	void _SocketCAN::update(void)
	{
		while (m_pT->bAlive())
		{
			if (!bOpen())
			{
				if (!open())
				{
					m_pT->sleepT(SEC_2_USEC);
					continue;
				}
			}

			if (m_iErr > m_nErrReconnect)
			{
				close();
			}

			m_pT->autoFPS();
		}
	}

	bool _SocketCAN::sendFrame(const CAN_F &f)
	{
		IF_F(!check());

		can_frame canF = {0};
		canF.can_id = f.m_ID;
		canF.len = f.m_nData;
		memcpy(canF.data, f.m_pData, f.m_nData);

		int nB = sizeof(can_frame);
		int nW = write(m_socket, &canF, nB); // f.m_nData);
		if (nB != nW)
		{
			LOG_E("write(can_frame)");
			m_iErr++;
			return false;
		}

		LOG_I("Sent: id=" + i2str(f.m_ID) + ", len=" + i2str(f.m_nData));
		return true;
	}

	bool _SocketCAN::readFrame(CAN_F *pF)
	{
		IF_F(!check());
		NULL_F(pF);

		can_frame canF = {0};

		int nB = sizeof(can_frame);
		int nR = read(m_socket, &canF, nB);
		if (nR < 0)
		{
			LOG_E("read(can_frame)");
			m_iErr++;
			return false;
		}

		pF->clear();
		pF->m_ID = canF.can_id & CAN_EFF_MASK;
		pF->m_nData = canF.len;
		memcpy(pF->m_pData, canF.data, canF.len);

		LOG_I("Recv: id=" + i2str(pF->m_ID) +
			  ", len=" + i2str(pF->m_nData) +
			  ", data=" + i2str(pF->m_pData[0]) +
			  ", " + i2str(pF->m_pData[1]) +
			  ", " + i2str(pF->m_pData[2]) +
			  ", " + i2str(pF->m_pData[3]) +
			  ", " + i2str(pF->m_pData[4]) +
			  ", " + i2str(pF->m_pData[5]) +
			  ", " + i2str(pF->m_pData[6]) +
			  ", " + i2str(pF->m_pData[7]));

		return true;
	}

	void _SocketCAN::handleFrame(const CAN_F &f)
	{
	}

	void _SocketCAN::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_CANbase::console(pConsole);

		_Console *pC = (_Console *)pConsole;
	}

}
