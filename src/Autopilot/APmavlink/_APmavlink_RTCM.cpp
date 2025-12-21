#include "_APmavlink_RTCM.h"

namespace kai
{

	_APmavlink_RTCM::_APmavlink_RTCM()
	{
		m_pMav = nullptr;
	}

	_APmavlink_RTCM::~_APmavlink_RTCM()
	{
	}

	int _APmavlink_RTCM::init(void *pKiss)
	{
		CHECK(this->_RTCMcast::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		return OK_OK;
	}

	int _APmavlink_RTCM::link(void)
	{
		CHECK_(this->_RTCMcast::link());
		Kiss *pK = (Kiss *)m_pKiss;
		string n;

		n = "";
		pK->v("_Mavlink", &n);
		m_pMav = (_Mavlink *)(pK->findModule(n));
		NULL__(m_pMav, OK_ERR_NOT_FOUND);

		return OK_OK;
	}

	int _APmavlink_RTCM::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		NULL__(m_pTr, OK_ERR_NULLPTR);
		CHECK_(m_pT->start(getUpdateW, this));
		return m_pTr->start(getUpdateR, this);
	}

	int _APmavlink_RTCM::check(void)
	{
		NULL__(m_pMav, OK_ERR_NULLPTR);

		return this->_ProtocolBase::check();
	}

	void _APmavlink_RTCM::updateW(void)
	{
		while (m_pT->bAlive())
		{
			sendMsg();

			m_pT->sleepT(0);
		}
	}

	void _APmavlink_RTCM::sendMsg(void)
	{
		IF_(check() != OK_OK);

		for (int i = 0; i < m_vMsg.size(); i++)
		{
			RTCM_MSG *pM = &m_vMsg[i];
			uint64_t tLr = pM->m_tLastRecv;

			IF_CONT(pM->m_tLastRecv == 0);
			IF_CONT(pM->m_tLastSent == tLr);

			IF_CONT(!writeMavlink(*pM));

			pM->m_tLastSent = tLr;
		}
	}

	bool _APmavlink_RTCM::writeMavlink(const RTCM_MSG &msg)
	{
		IF_F(check() != OK_OK);

		m_D.flags = 0;
		// TODO

		m_pMav->gpsRTCMdata(m_D);
	}

	void _APmavlink_RTCM::updateR(void)
	{
		RTCM_MSG rtcmMsg;

		while (m_pTr->bAlive())
		{
			IF_CONT(!readMsg(&rtcmMsg));

			handleMsg(rtcmMsg);
			rtcmMsg.init();
			m_nCMDrecv++;
		}
	}

	void _APmavlink_RTCM::handleMsg(const RTCM_MSG &msg)
	{
		uint64_t tNow = getTbootMs();

		for (int i = 0; i < m_vMsg.size(); i++)
		{
			RTCM_MSG *pM = &m_vMsg[i];
			IF_CONT(pM->m_msgID != msg.m_msgID);
			IF_(*pM == msg);

			pM->updateTo(msg);
			pM->m_tLastRecv = tNow;
			//TODO: wakeup send thread
			return;
		}

		RTCM_MSG m;
		m.init();
		m.m_msgID = msg.m_msgID;
		m.updateTo(msg);
		m.m_tLastRecv = tNow;
		m_vMsg.push_back(m);
		//TODO: wakeup send thread
	}

	void _APmavlink_RTCM::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_RTCMcast::console(pConsole);

		_Console *pC = (_Console *)pConsole;
		//		pC->addMsg("nCMD = " + i2str(m_nCMDrecv), 1);
	}

}
