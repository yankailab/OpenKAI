#include "_AP_swarm.h"

namespace kai
{

	_AP_swarm::_AP_swarm()
	{
		m_pAP = NULL;
		m_pXb = NULL;
		m_pSwarm = NULL;

		m_bAutoArm = true;
		m_altTakeoff = 5.0;
	}

	_AP_swarm::~_AP_swarm()
	{
	}

	bool _AP_swarm::init(void *pKiss)
	{
		IF_F(!this->_StateBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("bAutoArm", &m_bAutoArm);
		pK->v("altTakeoff", &m_altTakeoff);

		return true;
	}

	bool _AP_swarm::link(void)
	{
		IF_F(!this->_StateBase::link());
		Kiss *pK = (Kiss *)m_pKiss;

		IF_F(!m_pSC);
		IF_F(!m_state.assign(m_pSC));

		string n;

		n = "";
		pK->v("_AP_base", &n);
		m_pAP = (_AP_base *)(pK->getInst(n));
		IF_Fl(!m_pAP, n + ": not found");

		n = "";
		pK->v("_SwarmSearch", &n);
		m_pSwarm = (_SwarmSearch *)(pK->getInst(n));
		IF_Fl(!m_pSwarm, n + ": not found");

		n = "";
		pK->v("_Xbee", &n);
		m_pXb = (_Xbee *)(pK->getInst(n));
		IF_Fl(!m_pXb, n + ": not found");

		IF_F(!m_pXb->setCbReceivePacket(sOnRecvMsg, this));

		return true;
	}

	bool _AP_swarm::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	int _AP_swarm::check(void)
	{
		NULL__(m_pAP, -1);
		NULL__(m_pAP->m_pMav, -1);
		NULL__(m_pSC, -1);
		NULL__(m_pXb, -1);
		NULL__(m_pSwarm, -1);

		return this->_StateBase::check();
	}

	void _AP_swarm::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPSfrom();
			this->_StateBase::update();

			updateState();
			updateSwarm();

			m_pT->autoFPSto();
		}
	}

	void _AP_swarm::updateState(void)
	{
		IF_(check() < 0);
		m_state.update(m_pSC);

		int apMode = m_pAP->getApMode();
		bool bApArmed = m_pAP->bApArmed();
		float alt = m_pAP->getGlobalPos().w; // relative altitude

		// Standby
		if (m_state.bSTANDBY())
		{
			IF_(apMode != AP_COPTER_GUIDED);
		}

		// Takeoff
		if (m_state.bTAKEOFF())
		{
			m_pAP->setApMode(AP_COPTER_GUIDED);
			//			IF_(apMode != AP_COPTER_GUIDED);

			if (!bApArmed)
			{
				if (m_bAutoArm)
				{
					m_pAP->setApArm(true);
				}

				return;
			}

			m_pAP->m_pMav->clNavTakeoff(m_altTakeoff);
		}

		if (m_state.bAUTO())
		{
			IF_(apMode != AP_COPTER_GUIDED);
		}

		// RTL
		if (m_state.bRTL())
		{
			if (apMode == AP_COPTER_GUIDED)
				m_pAP->setApMode(AP_COPTER_RTL);

			// check if touched down
			IF_(bApArmed);
		}
	}

	void _AP_swarm::updateSwarm(void)
	{
		IF_(check() < 0);
		m_state.update(m_pSC);

		int apMode = m_pAP->getApMode();
		bool bApArmed = m_pAP->bApArmed();
		float alt = m_pAP->getGlobalPos().w; // relative altitude

	}

	void _AP_swarm::onRecvMsg(const XBframe_receivePacket &d)
	{
		uint8_t mType = d.m_pD[0];

		switch (mType)
		{
		case swMsg_hB:
			SWMSG_HB mHb;
			if (mHb.decode(d.m_pD, d.m_nD))
				m_pSwarm->handleMsgHB(mHb);
			break;
		case swMsg_cmd_setState:
			SWMSG_CMD_SETSTATE mSS;
			if (mSS.decode(d.m_pD, d.m_nD))
				handleMsgSetState(mSS);
			break;
		case swMsg_gc_update:
			SWMSG_GC_UPDATE mGU;
			if (mGU.decode(d.m_pD, d.m_nD))
				m_pSwarm->handleMsgGCupdate(mGU);
			break;
		}
	}

	void _AP_swarm::handleMsgSetState(const SWMSG_CMD_SETSTATE &m)
	{
		IF_(check() < 0);
		IF_((m.m_dstID != 0) && (m.m_dstID != m_pXb->getMyAddr()));

		//TODO: enable iMsg counter

		switch (m.m_state)
		{
		case swMsg_state_standBy:
			m_pSC->transit(m_state.STANDBY);
			break;
		case swMsg_state_takeOff:
			m_pSC->transit(m_state.TAKEOFF);
			break;
		case swMsg_state_auto:
			m_pSC->transit(m_state.AUTO);
			break;
		case swMsg_state_RTL:
			m_pSC->transit(m_state.RTL);
			break;
		case swMsg_state_land:
			m_pSC->transit(m_state.RTL);
			break;
		}
	}

}
