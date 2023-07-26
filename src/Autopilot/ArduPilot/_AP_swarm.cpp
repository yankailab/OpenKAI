#include "_AP_swarm.h"

namespace kai
{

	_AP_swarm::_AP_swarm()
	{
		m_pAP = NULL;
		m_pAfollow = NULL;
		m_pXb = NULL;
		m_pSwarm = NULL;

		m_bAutoArm = true;
		m_altTakeoff = 10.0;
		m_altAuto = 10.0;
		m_altLand = 8.0;
		m_myID = 0;
		m_vTargetID.clear();

		m_ieSendHB.init(USEC_1SEC);
		m_ieSendGCupdate.init(USEC_1SEC);
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
		pK->v("altAuto", &m_altAuto);
		pK->v("altLand", &m_altLand);
		pK->v("myID", &m_myID);
		pK->v("vTargetID", &m_vTargetID);

		pK->v("ieSendHB", &m_ieSendHB.m_tInterval);
		pK->v("ieSendGCupdate", &m_ieSendGCupdate.m_tInterval);

		return true;
	}

	bool _AP_swarm::link(void)
	{
		IF_F(!this->_StateBase::link());
		IF_F(!m_pSC);
		m_state.STANDBY = m_pSC->getStateIdxByName("STANDBY");
		m_state.TAKEOFF = m_pSC->getStateIdxByName("TAKEOFF");
		m_state.AUTO = m_pSC->getStateIdxByName("AUTO");
		m_state.RTL = m_pSC->getStateIdxByName("RTL");
		IF_F(!m_state.bValid());

		Kiss *pK = (Kiss *)m_pKiss;
		string n;

		n = "";
		pK->v("_AP_base", &n);
		m_pAP = (_AP_base *)(pK->getInst(n));
		IF_Fl(!m_pAP, n + ": not found");

		n = "";
		pK->v("_AP_follow", &n);
		m_pAfollow = (_AP_follow *)(pK->getInst(n));
		IF_Fl(!m_pAfollow, n + ": not found");

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
		NULL__(m_pAfollow, -1);
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

			send();

			m_pT->autoFPSto();
		}
	}

	void _AP_swarm::updateState(void)
	{
		IF_(check() < 0);

		int apMode = m_pAP->getApMode();
		bool bApArmed = m_pAP->bApArmed();
		float alt = m_pAP->getGlobalPos().w; // relative altitude

		m_state.update(m_pSC->getStateIdx());

		// Standby
		if (m_state.bSTANDBY())
		{
			// stop move etc.
			return;
		}

		// Takeoff
		if (m_state.bTAKEOFF())
		{
			if (apMode != AP_COPTER_GUIDED)
			{
				m_pAP->setApMode(AP_COPTER_GUIDED);
				return;
			}

			if (!bApArmed)
			{
				if (m_bAutoArm)
					m_pAP->setApArm(true);

				return;
			}

			if (alt > m_altTakeoff)
			{
				m_pSC->transit(m_state.AUTO);
				return;
			}

			// add some extra to make it easier for takeoff completion detection
			m_pAP->m_pMav->clNavTakeoff(m_altTakeoff + 1.0);
			return;
		}

		// Auto
		if (m_state.bAUTO())
		{
			IF_(apMode != AP_COPTER_GUIDED);
			IF_(!bApArmed);
			followTarget();
			return;
		}

		// RTL
		if (m_state.bRTL())
		{
			if (alt > m_altLand)
			{
				if (apMode == AP_COPTER_GUIDED)
					m_pAP->setApMode(AP_COPTER_RTL);

				return;
			}

			// check if touched down
			IF_(!bApArmed);

			// temporarily set to RTL
			if (apMode == AP_COPTER_GUIDED)
				m_pAP->setApMode(AP_COPTER_RTL);

			// TODO:enable vision navigated landing
		}
	}

	void _AP_swarm::followTarget(void)
	{
		IF_(check() < 0);

		SWARM_NODE *pN = m_pSwarm->getNodeByIDrange(m_vTargetID);
		if (!pN)
		{
			m_pAfollow->wakeUp();
			return;
		}

		if (!pN->m_bPosValid)
		{
			m_pAfollow->wakeUp();
			return;
		}

		m_pAfollow->goSleep();

		vDouble4 vP;
		vP.x = pN->m_vPos.x;
		vP.y = pN->m_vPos.y;
		vP.z = m_altAuto;
		vP.w = 0;
		m_pAfollow->setPglobal(vP, false, false);
	}

	void _AP_swarm::send(void)
	{
		IF_(check() < 0);

		uint64_t t = m_pT->getTfrom();

		if (m_ieSendHB.update(t))
			sendHB();
		// if (m_ieSendGCupdate.update(t))
		//     sendGCupdate();
	}

	void _AP_swarm::sendHB(void)
	{
		IF_(check() < 0);

		vDouble4 vP = m_pAP->getGlobalPos();
		SWMSG_HB m;
		m.m_srcID = m_myID;
		m.m_lat = vP.x * 1e7;
		m.m_lng = vP.y * 1e7;
		m.m_alt = vP.w * 1e2;
		m.m_hdg = m_pAP->getApHdg() * 1e1;
		m.m_spd = m_pAP->getApSpeed().len() * 1e2;
		m.m_batt = m_pAP->getBattery() * 1e2;
		m.m_mode = m_pSC->getStateIdx();

		uint8_t pB[XB_N_PAYLOAD];
		int nB = m.encode(pB, XB_N_PAYLOAD);
		IF_(nB <= 0);

		m_pXb->send(XB_BRDCAST_ADDR, pB, nB);
	}

	void _AP_swarm::sendGCupdate(void)
	{
		SWMSG_GC_UPDATE m;
		m.m_srcID = m_myID;
		m.m_dstID = 0;
		m.m_iGC = 0;
		m.m_w = 1;

		uint8_t pB[XB_N_PAYLOAD];
		int nB = m.encode(pB, XB_N_PAYLOAD);
		IF_(nB <= 0);

		m_pXb->send(XB_BRDCAST_ADDR, pB, nB);
	}

	void _AP_swarm::onRecvMsg(const XBframe_receivePacket &d)
	{
		uint8_t mType = d.m_pD[0];

		switch (mType)
		{
		case swMsg_hB:
		{
			SWMSG_HB mHb;
			mHb.m_srcNetAddr = d.m_srcAddr;
			if (mHb.decode(d.m_pD, d.m_nD))
				m_pSwarm->handleMsgHB(mHb);
		}
		break;
		case swMsg_cmd_setState:
		{
			SWMSG_CMD_SETSTATE mSS;
			mSS.m_srcNetAddr = d.m_srcAddr;
			if (mSS.decode(d.m_pD, d.m_nD))
				handleMsgSetState(mSS);
		}
		break;
		case swMsg_gc_update:
		{
			SWMSG_GC_UPDATE mGU;
			mGU.m_srcNetAddr = d.m_srcAddr;
			if (mGU.decode(d.m_pD, d.m_nD))
				m_pSwarm->handleMsgGCupdate(mGU);
		}
		break;
		}
	}

	void _AP_swarm::handleMsgSetState(const SWMSG_CMD_SETSTATE &m)
	{
		IF_(check() < 0);
		IF_((m.m_dstID != XB_BRDCAST_ADDR) && (m.m_dstID != m_pXb->getMyAddr()));

		// TODO: enable iMsg counter

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
