#include "_USR_CANET.h"

namespace kai
{

	_USR_CANET::_USR_CANET()
	{
		m_pTr = nullptr;
		m_pIO = nullptr;
		m_nFrameRecv = 0;
	}

	_USR_CANET::~_USR_CANET()
	{
	}

	int _USR_CANET::init(void *pKiss)
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

	int _USR_CANET::link(void)
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

	int _USR_CANET::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		NULL__(m_pTr, OK_ERR_NULLPTR);
		CHECK_(m_pT->start(getUpdateW, this));
		return m_pTr->start(getUpdateR, this);
	}

	int _USR_CANET::check(void)
	{
		NULL__(m_pIO, OK_ERR_NULLPTR);
		IF__(!m_pIO->bOpen(), OK_ERR_NOT_READY);

		return this->_ModuleBase::check();
	}

	void _USR_CANET::updateW(void)
	{
		while (m_pT->bAlive())
		{
			if (!m_pIO)
			{
				m_pT->sleepT(SEC_2_USEC);
				continue;
			}

			if (!m_pIO->bOpen())
			{
				if (!m_pIO->open())
				{
					m_pT->sleepT(SEC_2_USEC);
					continue;
				}
			}

			m_pT->autoFPS();


			CAN_FRAME f;
			int8_t pData[8];
			pData[0] = 0; //str
			pData[1] = 0; //f/b
			pData[2] = -92; //stop
			pData[3] = 0;
			pData[4] = -92;
			pData[5] = -92;
			pData[6] = 0;
			pData[7] = 0;

			f.encode(0x0F, 8, (uint8_t*)pData, false);

			sendFrame(&f);
		}
	}

	bool _USR_CANET::sendFrame(CAN_FRAME* pF)
	{
		NULL_F(pF);
		return m_pIO->write(pF->m_pB, CAN_BUF_N);
	}

	void _USR_CANET::updateR(void)
	{
		CAN_FRAME f;

		while (m_pTr->bAlive())
		{
//			m_pTr->autoFPS();

			IF_CONT(!readFrame(&f));

//			m_pTr->skipSleep();

			handleFrame(f);
			f.clear();
			m_nFrameRecv++;
		}
	}

	bool _USR_CANET::readFrame(CAN_FRAME *pFrame)
	{
		IF_F(check() != OK_OK);
		NULL_F(pFrame);

		return pFrame->read(m_pIO);
	}

	void _USR_CANET::handleFrame(const CAN_FRAME &frame)
	{
		LOG_I("Recv: ID=" + i2str(frame.m_ID) +
			", bExtended=" + i2str(frame.m_bExtended) +
			", bRTR=" + i2str(frame.m_bRTR) +
			", nData=" + i2str(frame.m_nData) +
			", Data=" + i2str(frame.m_pData[0]) +
			", " + i2str(frame.m_pData[1]) +
			", " + i2str(frame.m_pData[2]) +
			", " + i2str(frame.m_pData[3]) +
			", " + i2str(frame.m_pData[4]) +
			", " + i2str(frame.m_pData[5]) +
			", " + i2str(frame.m_pData[6]) +
			", " + i2str(frame.m_pData[7])
		);
	}

	void _USR_CANET::console(void *pConsole)
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
