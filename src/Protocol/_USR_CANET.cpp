#include "_USR_CANET.h"

namespace kai
{

	_USR_CANET::_USR_CANET()
	{
		m_pTr = nullptr;
		m_pIO = nullptr;
		m_nCMDrecv = 0;

		m_nRead = 0;
		m_iRead = 0;
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


		// m_nCanData = 0;
		// while (1)
		// {
		// 	IF__(m_nCanData >= N_CANDATA, OK_ERR_INVALID_VALUE);
		// 	Kiss *pAddr = pK->child(m_nCanData);
		// 	if (pAddr->empty())
		// 		break;

		// 	m_pCanData[m_nCanData].init();
		// 	IF__(!pAddr->v("addr", (int *)&(m_pCanData[m_nCanData].m_addr)), OK_ERR_NOT_FOUND);
		// 	m_nCanData++;
		// }

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

		}
	}

	void _USR_CANET::updateR(void)
	{
		CAN_FRAME f;

		while (m_pTr->bAlive())
		{
			m_pTr->autoFPS();

			IF_CONT(!readFrame(&f));

			m_pTr->skipSleep();

			handleFrame(f);
			f.clear();
			m_nCMDrecv++;
		}
	}

	bool _USR_CANET::readFrame(CAN_FRAME *pFrame)
	{
		IF_F(check() != OK_OK);
		NULL_F(pFrame);

		if (m_nRead == 0)
		{
			m_nRead = m_pIO->read(m_pBuf, CAN_BUF_N);
			IF_F(m_nRead <= 0);
			m_iRead = 0;
		}

		while (m_iRead < m_nRead)
		{
			bool r = pFrame->input(m_pBuf[m_iRead++]);
			if (m_iRead == m_nRead)
			{
				m_iRead = 0;
				m_nRead = 0;
			}

			IF__(r, true);
		}

		return false;
	}

	void _USR_CANET::handleFrame(const CAN_FRAME &frame)
	{
		// uint8_t cmd = m_recvMsg.m_pBuf[2];

		// if (cmd == CAN_CMD_CAN_SEND)
		// {
		// 	uint32_t *pAddr = (uint32_t *)(&m_recvMsg.m_pBuf[3]);
		// 	uint32_t addr = *pAddr;

		// 	for (int i = 0; i < m_nCanData; i++)
		// 	{
		// 		CAN_FRAME *pCan = &m_pCanData[i];
		// 		IF_CONT(pCan->m_addr != addr);

		// 		pCan->m_len = m_recvMsg.m_pBuf[7];
		// 		memcpy(pCan->m_pData, &m_recvMsg.m_pBuf[8], 8);

		// 		LOG_I("Updated data for CANID:" + i2str(addr));
		// 		return;
		// 	}

		// 	LOG_I("CANID:" + i2str(addr));
		// }
	}

	void _USR_CANET::send(unsigned long addr, unsigned char len, unsigned char *pData)
	{
		IF_(len + 8 > CAN_BUF_N);
		IF_(!m_pIO->bOpen());

		unsigned char pBuf[CAN_BUF_N];

		//Link header
		pBuf[0] = 0xFE;	   //Mavlink begin
		pBuf[1] = 5 + len; //Payload Length
//		pBuf[2] = CAN_CMD_CAN_SEND;

		//Payload from here
		//CAN addr
		pBuf[3] = (uint8_t)addr;
		pBuf[4] = (uint8_t)(addr >> 8);
		pBuf[5] = (uint8_t)(addr >> 16);
		pBuf[6] = (uint8_t)(addr >> 24);

		//CAN data len
		pBuf[7] = len;

		for (int i = 0; i < len; i++)
		{
			pBuf[i + 8] = pData[i];
		}
		//Payload to here

		m_pIO->write(pBuf, len + 8);
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
