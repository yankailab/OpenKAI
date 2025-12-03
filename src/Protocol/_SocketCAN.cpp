#include "_SocketCAN.h"

namespace kai
{

	_SocketCAN::_SocketCAN()
	{
		m_pTr = nullptr;
		m_pIO = nullptr;
		m_nFrameRecv = 0;
	}

	_SocketCAN::~_SocketCAN()
	{
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

		return OK_OK;
	}

	int _SocketCAN::link(void)
	{
		CHECK_(this->_ModuleBase::link());
		CHECK_(m_pTr->link());

		Kiss *pK = (Kiss *)m_pKiss;
		string n;

		n = "";
		pK->v("_IObase", &n);
		m_pIO = (_IObase *)(pK->findModule(n));
		NULL__(m_pIO, OK_ERR_NOT_FOUND);

		return OK_OK;
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
		NULL__(m_pIO, OK_ERR_NULLPTR);
		IF__(!m_pIO->bOpen(), OK_ERR_NOT_READY);

		return this->_ModuleBase::check();
	}

	void _SocketCAN::updateW(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

		}
	}

	bool _SocketCAN::sendFrame(SOCKETCAN_FRAME* pF)
	{
		IF_F(check() != OK_OK);
		NULL_F(pF);

		return true;//m_pIO->write(pF->m_pB, CAN_BUF_N);
	}

	void _SocketCAN::updateR(void)
	{
		SOCKETCAN_FRAME f;

		while (m_pTr->bAlive())
		{
			// blocking read
			IF_CONT(!readFrame(&f));

			handleFrame(f);
			m_nFrameRecv++;
		}
	}

	bool _SocketCAN::readFrame(SOCKETCAN_FRAME *pF)
	{
		IF_F(check() != OK_OK);
		NULL_F(pF);

		return pF->read(m_pIO);
	}

	void _SocketCAN::handleFrame(const SOCKETCAN_FRAME &f)
	{
		LOG_I("Recv: ID=" + i2str(f.m_ID) +
			", bExtended=" + i2str(f.m_bExtended) +
			", bRTR=" + i2str(f.m_bRTR) +
			", nData=" + i2str(f.m_nData) +
			", Data=" + i2str(f.m_pData[0]) +
			", " + i2str(f.m_pData[1]) +
			", " + i2str(f.m_pData[2]) +
			", " + i2str(f.m_pData[3]) +
			", " + i2str(f.m_pData[4]) +
			", " + i2str(f.m_pData[5]) +
			", " + i2str(f.m_pData[6]) +
			", " + i2str(f.m_pData[7])
		);
	}

	void _SocketCAN::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);

		string msg = this->getName();
		if (m_pIO->bOpen())
			msg += ": CONNECTED";
		else
			msg += ": Not connected";

		((_Console *)pConsole)->addMsg(msg);
	}

}
