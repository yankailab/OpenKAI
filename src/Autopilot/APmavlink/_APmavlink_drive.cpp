#include "_APmavlink_drive.h"

namespace kai
{

	_APmavlink_drive::_APmavlink_drive()
	{
	}

	_APmavlink_drive::~_APmavlink_drive()
	{
	}

	bool _APmavlink_drive::init(const json &j)
	{
		IF_F(!this->_APmavlink_move::init(j));

		jKv(j, "steer", m_steer);
		jKv(j, "speed", m_speed);
		jKv(j, "pwmM", m_pwmM);
		jKv(j, "pwmD", m_pwmD);
		jKv(j, "iRCsteer", m_iRCsteer);
		jKv(j, "iRCthrottle", m_iRCthrottle);

		jKv(j, "tOutBtn", m_tOutBtn);
		jKv(j, "apModeMove", m_apModeMove);
		jKv(j, "octGridOccu", m_octGridOccu);
		jKv(j, "speedGo", m_speedGo);
		jKv(j, "steerTurn", m_steerTurn);

		return true;
	}

	bool _APmavlink_drive::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_APmavlink_move::link(j, pM));

		string n;

		n = "";
		jKv(j, "_OctreeGrid", n);
		m_pOctGrid = (_OctreeGrid *)(pM->findModule(n));
		IF_Le_F(!m_pOctGrid, "_OctreeGrid not found: " + n);

		n = "";
		jKv(j, "_GeoFence", n);
		m_pGfence = (_GeoFence *)(pM->findModule(n));
		IF_Le_F(!m_pGfence, "_GeoFence not found: " + n);

		return true;
	}

	bool _APmavlink_drive::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	bool _APmavlink_drive::check(void)
	{
		NULL_F(m_pOctGrid);
		NULL_F(m_pGfence);

		return this->_APmavlink_move::check();
	}

	void _APmavlink_drive::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			if (updateCtrl())
			{
				updateDrive();
			}

			ON_PAUSE;
		}
	}

	void _APmavlink_drive::onPause(void)
	{
		releaseRCoverride();
	}

	bool _APmavlink_drive::updateCtrl(void)
	{
		IF_F(!check());

		// check btn ctrl
		if (getTbootUs() - m_tLastBtn > m_tOutBtn)
		{
			m_btnPressed = apDrive_btnNone;
		}

		// switch mode
		if (m_btnPressed == apDrive_btnNone)
		{
			if (m_dMode == apDrive_modeManual)
				m_dMode = apDrive_modeStandby;
		}
		else if (m_btnPressed == apDrive_btnStop)
		{
			m_dMode = apDrive_modeStandby;
		}
		else
		{
			m_dMode = apDrive_modeManual;
		}

		// find obstacle
		bool bObstacle = false;
		for (const auto &id : m_pOctGrid->getSelectedCells())
		{
			const auto *pCell = m_pOctGrid->getCell(id);
			if (pCell && pCell->m_nP > m_octGridOccu)
			{
				bObstacle = true;
				break;
			}
		}

		// check fence breach
		bool bFenceBreach = false;
		if (m_pGfence)
		{
			vDouble4 vPos = m_pAP->getGlobalPos();
			float hdg = m_pAP->getHdg();
			m_pGfence->setPosHdg(vDouble2(vPos.x, vPos.y), hdg);

			bFenceBreach = m_pGfence->bBreach();
		}

		// stop in Standby mode
		if (m_dMode == apDrive_modeStandby)
		{
			setSteerSpeed(0, 0);
			return true;
		}

		// check AP mode and arming
		if (m_pAP->getMode() == m_apModeMove)
		{
			if (!m_pAP->bArmed())
				m_pAP->setArm(true);
		}
		else
		{
			m_pAP->setMode(m_apModeMove);
		}

		// move
		if (m_dMode == apDrive_modeManual)
		{
			if (m_btnPressed == apDrive_btnForward)
				setSteerSpeed(0, m_speedGo);
			else if (m_btnPressed == apDrive_btnBackward)
				setSteerSpeed(0, -m_speedGo);
			else if (m_btnPressed == apDrive_btnLeft)
				setSteerSpeed(-m_steerTurn, 0);
			else if (m_btnPressed == apDrive_btnRight)
				setSteerSpeed(m_steerTurn, 0);
		}
		else if (m_dMode == apDrive_modeAuto)
		{
			if (!bObstacle && !bFenceBreach)
				setSteerSpeed(0, m_speedGo);
			else
				setSteerSpeed(m_steerTurn, 0);
		}

		return true;
	}

	void _APmavlink_drive::setSteerSpeed(float steer, float spd)
	{
		m_steer = steer;
		m_speed = spd;
	}

	void _APmavlink_drive::updateDrive(void)
	{
		setRCchan(m_iRCsteer, constrain(m_steer * m_pwmD + m_pwmM, m_pwmM - m_pwmD, m_pwmM + m_pwmD), false);
		setRCchan(m_iRCthrottle, constrain(m_speed * m_pwmD + m_pwmM, m_pwmM - m_pwmD, m_pwmM + m_pwmD), true); // flash cmd to AP mavlink
	}

	void _APmavlink_drive::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_APmavlink_move::console(pConsole);

		_Console *pC = (_Console *)pConsole;
		pC->addMsg("steer = " + f2str(m_steer) + ", speed = " + f2str(m_speed));
		pC->addMsg("btnPressed = " + i2str(m_btnPressed));
		pC->addMsg("dMode = " + i2str(m_dMode));
	}

	void _APmavlink_drive::console(const json &j, void *pJSONbase)
	{
		_JSONbase *pJb = (_JSONbase *)pJSONbase;
		string cmd;
		IF_(!jKv(j, "cmd", cmd));

		if (cmd == "setSteerSpeed")
		{
			jKv(j, "steer", m_steer);
			jKv(j, "speed", m_speed);

			setSteerSpeed(m_steer, m_speed);

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
		else if (cmd == "startAuto")
		{
			m_dMode = apDrive_modeAuto;

			NULL_(pJb);
			json jr = json::object();
			jr["cmd"] = "startAuto";
			jr["bSuccess"] = true;
			pJb->sendJson(jr);
		}
	}

}
