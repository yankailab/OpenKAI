#include "_APmavlink_swarm.h"

namespace kai
{

	_APmavlink_swarm::_APmavlink_swarm()
	{
		m_pAP = nullptr;
		m_pXb = nullptr;
		m_pSwarm = nullptr;
		m_pGio = nullptr;
		m_pU = nullptr;

		m_bAutoArm = true;
		m_altTakeoff = 10.0;
		m_altAuto = 10.0;
		m_altLand = 8.0;
		m_myID = 0;

		m_vTargetID.clear();
		m_iClass = 1;
		m_dRdet = 1e-6;

		m_ieNextWP.init(USEC_10SEC);
		m_vWP.set(0, 0);
		m_vWPd.set(5, 5);
		m_vWPrange.set(10, 10);

		m_ieSendHB.init(USEC_1SEC);
		m_ieSendGCupdate.init(USEC_1SEC);
	}

	_APmavlink_swarm::~_APmavlink_swarm()
	{
	}

	int _APmavlink_swarm::init(const json& j)
	{
		CHECK_(this->_APmavlink_move::init(j));

		= j.value("bAutoArm", &m_bAutoArm);
		= j.value("altTakeoff", &m_altTakeoff);
		= j.value("altAuto", &m_altAuto);
		= j.value("altLand", &m_altLand);
		= j.value("myID", &m_myID);

		= j.value("vTargetID", &m_vTargetID);
		= j.value("iClass", &m_iClass);
		= j.value("dRdet", &m_dRdet);

		= j.value("vWPd", &m_vWPd);
		= j.value("vWPrange", &m_vWPrange);
		= j.value("ieNextWP", &m_ieNextWP.m_tInterval);

		= j.value("ieSendHB", &m_ieSendHB.m_tInterval);
		= j.value("ieSendGCupdate", &m_ieSendGCupdate.m_tInterval);

		return true;
	}

	int _APmavlink_swarm::link(const json& j, ModuleMgr* pM)
	{
		CHECK_(this->_APmavlink_move::link(j, pM));

		string n;

        n = "";
        = j.value("_StateControl", &n);
        m_pSC = (_StateControl *)(pM->findModule(n));
        NULL__(m_pSC);

		m_state.STANDBY = m_pSC->getStateIdxByName("STANDBY");
		m_state.TAKEOFF = m_pSC->getStateIdxByName("TAKEOFF");
		m_state.AUTO = m_pSC->getStateIdxByName("AUTO");
		m_state.RTL = m_pSC->getStateIdxByName("RTL");
		IF__(!m_state.bValid(), OK_ERR_INVALID_VALUE);

		n = "";
		= j.value("_APmavlink_base", &n);
		m_pAP = (_APmavlink_base *)(pM->findModule(n));
        NULL__(m_pAP);

		n = "";
		= j.value("_Swarm", &n);
		m_pSwarm = (_SwarmSearch *)(pM->findModule(n));
        NULL__(m_pSwarm);

		n = "";
		= j.value("_Xbee", &n);
		m_pXb = (_Xbee *)(pM->findModule(n));
        NULL__(m_pXb);

		n = "";
		= j.value("_IObase", &n);
		m_pGio = (_IObase *)(pM->findModule(n));
        NULL__(m_pGio);

		n = "";
		= j.value("_Universe", &n);
		m_pU = (_Universe *)(pM->findModule(n));
        NULL__(m_pU);

		IF__(!m_pXb->setCbReceivePacket(sOnRecvMsg, this), OK_ERR_INVALID_VALUE);

		return true;
	}

	int _APmavlink_swarm::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	int _APmavlink_swarm::check(void)
	{
		NULL__(m_pAP);
		NULL__(m_pAP->getMavlink());
		NULL__(m_pXb);
		NULL__(m_pSwarm);
		NULL__(m_pSC);
		NULL__(m_pU);

		return this->_APmavlink_move::check();
	}

	void _APmavlink_swarm::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			updateState();

			send();

		}
	}

	void _APmavlink_swarm::updateState(void)
	{
		IF_(!check());

		int apMode = m_pAP->getMode();
		bool bApArmed = m_pAP->bApArmed();
		float alt = m_pAP->getGlobalPos().w; // relative altitude

		m_state.update(m_pSC->getCurrentStateIdx());

		// Standby
		if (m_state.bSTANDBY())
		{
			setHold();
			return;
		}

		// Takeoff
		if (m_state.bTAKEOFF())
		{
			if (apMode != AP_COPTER_GUIDED)
			{
				m_pAP->setMode(AP_COPTER_GUIDED);
				return;
			}

			if (!bApArmed)
			{
				if (m_bAutoArm)
					m_pAP->setArm(true);

				return;
			}

			if (alt > m_altTakeoff)
			{
				m_pSC->transit(m_state.AUTO);
				return;
			}

			// add some extra to make it easier for takeoff completion detection
			m_pAP->getMavlink()->clNavTakeoff(m_altTakeoff + 1.0);
			return;
		}

		// Auto
		if (m_state.bAUTO())
		{
			IF_(apMode != AP_COPTER_GUIDED);

			IF_(findBeacon());
			if (!findVisual())
			{
				calcMove();
				return;
			}

			setHold();

			return;
		}

		// RTL
		if (m_state.bRTL())
		{
			if (apMode == AP_COPTER_GUIDED)
				m_pAP->setMode(AP_COPTER_RTL);
		}
	}

	bool _APmavlink_swarm::findBeacon(void)
	{
		IF_F(!check());

		SWARM_NODE *pN = m_pSwarm->getNodeByIDrange(m_vTargetID);
		NULL_F(pN);
		IF_F(!pN->m_bPosValid);

		vDouble4 vP;
		vP.x = pN->m_vPos.x;
		vP.y = pN->m_vPos.y;
		vP.z = m_altAuto;
		vP.w = 0;
		setPglobal(vP, false, false);

		// SWARM_DETECTION *pD = getDetByDist(pN->m_vPos, m_dRdet);
		// IF__(pD);

		// SWARM_DETECTION d;
		// d.m_id = pN->m_id;
		// d.m_vPos = pN->m_vPos;
		// m_vDetections.push_back(d);

		return true;
	}

	bool _APmavlink_swarm::findVisual(void)
	{
		IF_F(!check());

		gimbalDownward();

		IF_F(!findVisualTarget());

		vDouble4 vPos = m_pAP->getGlobalPos();
		vDouble2 vP;
		vP.x = vPos.x;
		vP.y = vPos.y;
		SWARM_DETECTION *pD = getDetByDist(vP, m_dRdet);
		IF__(pD, true);

		pD = getDetByID(m_myID + 200);
		if (pD)
		{
			pD->m_vPos = vP;
			return true;
		}

		SWARM_DETECTION d;
		d.m_id = m_myID + 200;
		d.m_vPos = vP;
		m_vDetections.push_back(d);

		return true;
	}

	bool _APmavlink_swarm::findVisualTarget(void)
	{
		IF_F(!check());

		_Object *pO;
		int i = 0;
		while ((pO = m_pU->get(i++)) != NULL)
		{
			IF_CONT(pO->getTopClass() != m_iClass);

			return true;
		}

		return false;
	}

	void _APmavlink_swarm::calcMove(void)
	{
		uint64_t t = getApproxTbootUs();
		IF_(!m_ieSendHB.update(t));

		m_vWP += m_vWPd;

		vFloat4 vWP;
		vWP.x = m_vWP.x;
		vWP.y = m_vWP.y;
		vWP.z = 0;
		vWP.w = 0;
		setPlocal(vWP);

		if (m_vWP.x >= m_vWPrange.x)
			m_vWPd.x = -abs(m_vWPd.x);
		else if (m_vWP.x <= -m_vWPrange.x)
			m_vWPd.x = abs(m_vWPd.x);

		if (m_vWP.y >= m_vWPrange.y)
			m_vWPd.y = -abs(m_vWPd.y);
		else if (m_vWP.y <= -m_vWPrange.y)
			m_vWPd.y = abs(m_vWPd.y);
	}

	SWARM_DETECTION *_APmavlink_swarm::getDetByID(uint16_t id)
	{
		for (int i = 0; i < m_vDetections.size(); i++)
		{
			SWARM_DETECTION *pD = &m_vDetections[i];
			IF_CONT(pD->m_id != id);

			return pD;
		}

		return NULL;
	}

	SWARM_DETECTION *_APmavlink_swarm::getDetByDist(vDouble2 vP, double r)
	{
		for (int i = 0; i < m_vDetections.size(); i++)
		{
			SWARM_DETECTION *pD = &m_vDetections[i];

			vDouble2 vD = pD->m_vPos - vP;
			IF_CONT(vD.len() > r);

			return pD;
		}

		return NULL;
	}

	void _APmavlink_swarm::gimbalDownward(void)
	{
		NULL_(m_pGio);
		IF_(!m_pGio->bOpen());

		string cmd = "#TPUG2wPTZ02";
		char crc = calcCRC(cmd.c_str(), (int)cmd.length());

		char buf[3];
		snprintf(buf, 3, "%X", crc);
		cmd += string(buf);

		m_pGio->write((uint8_t *)cmd.c_str(), (int)cmd.length());
	}

	char _APmavlink_swarm::calcCRC(const char *cmd, uint8_t len)
	{
		char crc = 0;
		for (int i = 0; i < len; i++)
		{
			crc += cmd[i];
		}
		return crc;
	}

	void _APmavlink_swarm::send(void)
	{
		IF_(!check());

		uint64_t t = m_pT->getTfrom();

		if (m_ieSendHB.update(t))
		{
			sendHB();
			sendDetectionHB();
		}

		// if (m_ieSendGCupdate.update(t))
		//     sendGCupdate();
	}

	void _APmavlink_swarm::sendHB(void)
	{
		IF_(!check());

		vDouble4 vP = m_pAP->getGlobalPos();
		SWMSG_HB m;
		m.m_srcID = m_myID;
		m.m_lat = vP.x * 1e7;
		m.m_lng = vP.y * 1e7;
		m.m_alt = vP.w * 1e2;
		m.m_hdg = m_pAP->getHdg() * 1e1;
		m.m_spd = m_pAP->getSpeed().len() * 1e2;
		m.m_batt = m_pAP->getBattery() * 1e2;
		m.m_mode = m_pSC->getCurrentStateIdx();

		uint8_t pB[XB_N_PAYLOAD];
		int nB = m.encode(pB, XB_N_PAYLOAD);
		IF_(nB <= 0);

		m_pXb->send(XB_BRDCAST_ADDR, pB, nB);
	}

	void _APmavlink_swarm::sendDetectionHB(void)
	{
		IF_(!check());

		for (int i = 0; i < m_vDetections.size(); i++)
		{
			SWARM_DETECTION *pD = &m_vDetections[i];

			SWMSG_HB m;
			m.m_srcID = pD->m_id;
			m.m_lat = pD->m_vPos.x * 1e7;
			m.m_lng = pD->m_vPos.y * 1e7;
			uint8_t pB[XB_N_PAYLOAD];
			int nB = m.encode(pB, XB_N_PAYLOAD);
			IF_CONT(nB <= 0);

			m_pXb->send(XB_BRDCAST_ADDR, pB, nB);
		}
	}

	void _APmavlink_swarm::sendGCupdate(void)
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

	void _APmavlink_swarm::onRecvMsg(const XBframe_receivePacket &d)
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

	void _APmavlink_swarm::handleMsgSetState(const SWMSG_CMD_SETSTATE &m)
	{
		IF_(!check());
		IF_((m.m_dstID != XB_BRDCAST_ADDR) && (m.m_dstID != m_pXb->getMyAddr()));

		m_state.update(m_pSC->getCurrentStateIdx());

		// TODO: enable iMsg counter

		switch (m.m_state)
		{
		case swMsg_state_standBy:
			m_pSC->transit(m_state.STANDBY);
			m_vWP.set(0, 0);
			break;
		case swMsg_state_takeOff:
			if (m_state.bSTANDBY())
			{
				m_pSC->transit(m_state.TAKEOFF);
			}
			break;
		case swMsg_state_auto:
			if (!m_state.bRTL())
			{
				m_pSC->transit(m_state.AUTO);
			}
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
