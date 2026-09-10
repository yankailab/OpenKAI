#include "_APmavlink_drive.h"

namespace kai
{

	_APmavlink_drive::_APmavlink_drive()
	{
		m_pAP = nullptr;
		m_pRcYaw = nullptr;
		m_pRcThrottle = nullptr;

		m_yawMode = 1.0;
		m_bSetYawSpeed = false;
		m_bRcChanOverride = false;

		m_steer = 0.0;
		m_speed = 0.0;
		m_pwmM = 1500;
		m_pwmD = 500;

		m_btnPressed = apDrive_btnNone;
		m_tLastBtn = 0;
		m_tOutBtn = 100000;

		m_pGfence = nullptr;
		m_pOctGrid = nullptr;
		m_octGridOccu = 1;
		m_speedGo = 0.2;
		m_steerTurn = 0.2;
	}

	_APmavlink_drive::~_APmavlink_drive()
	{
	}

	bool _APmavlink_drive::init(const json &j)
	{
		IF_F(!this->_ModuleBase::init(j));

		jKv(j, "bSetYawSpeed", m_bSetYawSpeed);
		jKv(j, "yawMode", m_yawMode);
		jKv(j, "bRcChanOverride", m_bRcChanOverride);

		jKv(j, "steer", m_steer);
		jKv(j, "speed", m_speed);
		jKv(j, "pwmM", m_pwmM);
		jKv(j, "pwmD", m_pwmD);
		jKv(j, "tOutBtn", m_tOutBtn);

		jKv(j, "speedGo", m_speedGo);
		jKv(j, "steerTurn", m_steerTurn);
		jKv(j, "octGridOccu", m_octGridOccu);


		uint16_t *pRC[19];
		pRC[0] = NULL;
		pRC[1] = &m_rcOverride.chan1_raw;
		pRC[2] = &m_rcOverride.chan2_raw;
		pRC[3] = &m_rcOverride.chan3_raw;
		pRC[4] = &m_rcOverride.chan4_raw;
		pRC[5] = &m_rcOverride.chan5_raw;
		pRC[6] = &m_rcOverride.chan6_raw;
		pRC[7] = &m_rcOverride.chan7_raw;
		pRC[8] = &m_rcOverride.chan8_raw;
		pRC[9] = &m_rcOverride.chan9_raw;
		pRC[10] = &m_rcOverride.chan10_raw;
		pRC[11] = &m_rcOverride.chan11_raw;
		pRC[12] = &m_rcOverride.chan12_raw;
		pRC[13] = &m_rcOverride.chan13_raw;
		pRC[14] = &m_rcOverride.chan14_raw;
		pRC[15] = &m_rcOverride.chan15_raw;
		pRC[16] = &m_rcOverride.chan16_raw;
		pRC[17] = &m_rcOverride.chan17_raw;
		pRC[18] = &m_rcOverride.chan18_raw;

		for (int i = 1; i < 19; i++)
			*pRC[i] = UINT16_MAX;

		int iRcYaw = 1;
		jKv(j, "iRcYaw", iRcYaw);
		IF_F(iRcYaw <= 0 || iRcYaw > 18);
		m_pRcYaw = pRC[iRcYaw];

		int iRcThrottle = 3;
		jKv(j, "iRcThrottle", iRcThrottle);
		IF_F(iRcThrottle <= 0 || iRcThrottle > 18);
		m_pRcThrottle = pRC[iRcThrottle];

		return true;
	}

	bool _APmavlink_drive::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_ModuleBase::link(j, pM));

		string n = "";
		jKv(j, "_APmavlink_base", n);
		m_pAP = (_APmavlink_base *)(pM->findModule(n));
		IF_Le_F(!m_pAP, "_APmavlink_base not found: " + n);

        n = "";
        jKv(j, "_OctreeGrid", n);
        m_pOctGrid = (_OctreeGrid *)(pM->findModule(n));
        NULL_F(m_pOctGrid);

        n = "";
        jKv(j, "_GeoFence", n);
        m_pGfence = (_GeoFence *)(pM->findModule(n));
        NULL_F(m_pGfence);

		return true;
	}

	bool _APmavlink_drive::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	bool _APmavlink_drive::check(void)
	{
		NULL_F(m_pAP);
		NULL_F(m_pAP->getMavlink());

		return this->_ModuleBase::check();
	}

	void _APmavlink_drive::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			updateDrive();

			ON_PAUSE;
		}
	}

	void _APmavlink_drive::onPause(void)
	{
		IF_(!m_bRcChanOverride);

		*m_pRcYaw = 0;
		*m_pRcThrottle = 0;
		m_pAP->getMavlink()->rcChannelsOverride(m_rcOverride);
	}

    bool _APmavlink_drive::updateCtrl(void)
    {
        IF_F(!check());

        _Mavlink *pMav = m_pAP->getMavlink();

		bool bObstacle = false;
		// TODO: read the selected cell list from m_pOctGrid, iterate over them to see if any cell has nP > m_octGridOccu, if yes bObstacle = true
		//m_pOctGrid->getSelectedCells()

		bool bFenceBreach = false;
        if (m_pGfence)
        {
            vDouble4 vPos = m_pAP->getGlobalPos();
            float hdg = m_pAP->getHdg();
            m_pGfence->setPosHdg(vDouble2(vPos.x, vPos.y), hdg);

            bFenceBreach = m_pGfence->bBreach();
        }


		//TODO: button ctrl to speed and steer



        if (!bObstacle && !bFenceBreach)
        {
            // no obstacle nor fence breach, just Go
            setSteerSpeed(0, m_speedGo);
        }
        else
        {
            // obstacle on the way or fence breach, make turn
            setSteerSpeed(m_steerTurn, 0);
        }

        return true;
    }

	bool _APmavlink_drive::updateDrive(void)
	{
		if (getTbootUs() - m_tLastBtn > m_tOutBtn)
			m_btnPressed = apDrive_btnNone;

		IF_F(!check());

		if (m_bSetYawSpeed)
		{
			m_pAP->getMavlink()->clNavSetYawSpeed(m_steer,
												  m_speed,
												  m_yawMode);
		}

		if (m_bRcChanOverride)
		{
			if (m_pRcYaw)
			{
				*m_pRcYaw = constrain(m_steer * m_pwmD + m_pwmM,
									  m_pwmM - m_pwmD,
									  m_pwmM + m_pwmD);
			}

			if (m_pRcThrottle)
			{
				*m_pRcThrottle = constrain(m_speed * m_pwmD + m_pwmM,
										   m_pwmM - m_pwmD,
										   m_pwmM + m_pwmD);
			}

			m_pAP->getMavlink()->rcChannelsOverride(m_rcOverride);
		}

		return true;
	}

	void _APmavlink_drive::setSteerSpeed(float steer, float spd)
	{
		m_steer = steer;
		m_speed = spd;
	}

	void _APmavlink_drive::setYawMode(bool bRelative)
	{
		if (bRelative)
			m_yawMode = 1.0;
		else
			m_yawMode = 0.0;
	}

	void _APmavlink_drive::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);

		((_Console *)pConsole)->addMsg("steer=" + f2str(m_steer) + ", speed=" + f2str(m_speed));
		((_Console *)pConsole)->addMsg("btnPressed=" + i2str(m_btnPressed));

		NULL_(m_pRcYaw);
		NULL_(m_pRcThrottle);
		((_Console *)pConsole)->addMsg("yawMode=" + f2str(m_yawMode) + ", yaw=" + i2str(*m_pRcYaw) + ", throttle=" + i2str(*m_pRcThrottle));
	}

	void _APmavlink_drive::console(const json &j, void *pJSONbase)
	{
		_JSONbase *pJb = (_JSONbase *)pJSONbase;
		string cmd;
		IF_(!jKv(j, "cmd", cmd));

		if (cmd == "setSteerSpeed")
		{
			float steer = 0;
			jKv(j, "steer", steer);
			float speed = 0;
			jKv(j, "speed", speed);

			setSteerSpeed(steer, speed);

			NULL_(pJb);
			json jr = json::object();
			jr["cmd"] = "setSteerSpeed";
			jr["bSuccess"] = true;
			pJb->sendJson(jr);
		}
		else if (cmd == "ctrlBtn")
		{
			string btn;
			jKv(j, "btn", btn);

			AP_DRIVE_BTN pressed = apDrive_btnNone;
			if (btn == "F")
				pressed = apDrive_btnForward;
			else if (btn == "L")
				pressed = apDrive_btnLeft;
			else if (btn == "R")
				pressed = apDrive_btnRight;
			else if (btn == "B")
				pressed = apDrive_btnBackward;
			else if (btn == "S")
				pressed = apDrive_btnStop;

			const bool bSuccess = pressed != apDrive_btnNone;
			if (bSuccess)
			{
				m_btnPressed = pressed;
				m_tLastBtn = getTbootUs();
			}

			NULL_(pJb);
			json jr = json::object();
			jr["cmd"] = "ctrlBtn";
			jr["bSuccess"] = bSuccess;
			pJb->sendJson(jr);
		}
	}

}
