#include "_SocketCAN.h"

namespace kai
{

	_SocketCAN::_SocketCAN()
	{
		m_pTr = nullptr;

		m_ifName = "can0";
		m_socket = -1;

		m_bOpen = false;
		m_nFrameRecv = 0;
	}

	_SocketCAN::~_SocketCAN()
	{
		close();
		DEL(m_pTr);
	}

	int _SocketCAN::init(void *pKiss)
	{
		CHECK_(this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		Kiss *pKt = pK->child("threadR");
		if (pKt->empty())
		{
			LOG_E("threadR not found");
			return OK_ERR_NOT_FOUND;
		}

		m_pTr = new _Thread();
		CHECK_d_l_(m_pTr->init(pKt), DEL(m_pTr), "threadR init failed");

		pK->v("ifName", &m_ifName);

		return OK_OK;
	}

	int _SocketCAN::link(void)
	{
		CHECK_(this->_ModuleBase::link());
		CHECK_(m_pTr->link());

		Kiss *pK = (Kiss *)m_pKiss;
		string n;

		return OK_OK;
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

	bool _SocketCAN::bOpen(void)
	{
		return m_bOpen;
	}

	void _SocketCAN::close(void)
	{
		::close(m_socket);
		m_bOpen = false;
	}

	int _SocketCAN::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		NULL__(m_pTr, OK_ERR_NULLPTR);
		CHECK_(m_pT->start(getUpdateW, this));
		return m_pTr->start(getUpdateR, this);
	}

	int _SocketCAN::check(void)
	{
		IF__(!m_bOpen, OK_ERR_NOT_READY);

		return this->_ModuleBase::check();
	}

	void _SocketCAN::updateW(void)
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

			m_pT->autoFPS();
		}
	}

	bool _SocketCAN::sendFrame(const can_frame &f, int nB)
	{
		IF_F(check() != OK_OK);

		// --- Send one standard (11-bit) frame ---
		// struct can_frame tx = {0};
		// tx.can_id = 0x123; // standard ID
		// tx.can_dlc = 8;
		// tx.data[0] = 0x11;
		// tx.data[1] = 0x22;
		// tx.data[2] = 0x33;
		// tx.data[3] = 0x44;
		// tx.data[4] = 0x55;
		// tx.data[5] = 0x66;
		// tx.data[6] = 0x77;
		// tx.data[7] = 0x88;

		int nW = write(m_socket, &f, nB);
		if (nB != nW)
		{
			LOG_E("write(can_frame)");
			close();
			return false;
		}

		printf("Sent: id=0x%X dlc=%d\n", f.can_id, f.can_dlc);
		return true;
	}

	void _SocketCAN::updateR(void)
	{
		can_frame f;

		while (m_pTr->bAlive())
		{
			// blocking read
			if (!readFrame(&f))
			{
				m_pTr->sleepT(SEC_2_USEC);
				continue;
			}

			handleFrame(f);
			m_nFrameRecv++;
		}
	}

	bool _SocketCAN::readFrame(can_frame *pF, int nB)
	{
		IF_F(check() != OK_OK);
		NULL_F(pF);

		int nR = read(m_socket, pF, nB);
		if (nR < 0)
		{
			LOG_E("read(can_frame)");
			close();
			return false;
		}

		if (nR != nB)
		{
			LOG_I("nB=" + i2str(nB) + ", nR=" + i2str(nR));
		}

		return true;
	}

	void _SocketCAN::handleFrame(const can_frame &f)
	{
		printf("Recv: id=0x%X dlc=%d data=", f.can_id & CAN_EFF_MASK, f.can_dlc);
		for (int i = 0; i < f.can_dlc; i++)
			printf("%02X", f.data[i]);
		printf("\n");

		LOG_I("Recv: ID=" + i2str(f.can_id) +
			  ", bExtended=" + i2str(f.can_dlc) +
			  ", bRTR=" + i2str(f.can_dlc) +
			  ", " + i2str(f.data[0]) +
			  ", " + i2str(f.data[1]) +
			  ", " + i2str(f.data[2]) +
			  ", " + i2str(f.data[3]) +
			  ", " + i2str(f.data[4]) +
			  ", " + i2str(f.data[5]) +
			  ", " + i2str(f.data[6]) +
			  ", " + i2str(f.data[7]));
	}

	void _SocketCAN::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);

		_Console *pC = (_Console *)pConsole;
		pC->addMsg("nFrameRecv = " + i2str(m_nFrameRecv), 1);
	}

}
