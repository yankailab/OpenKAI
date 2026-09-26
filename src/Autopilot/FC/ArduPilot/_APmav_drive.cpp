#include "_APmav_drive.h"

namespace kai
{

	_APmav_drive::_APmav_drive()
	{
	}

	_APmav_drive::~_APmav_drive()
	{
	}

	bool _APmav_drive::loadConfig(void)
	{
		IF_F(!this->_APmav_move::loadConfig());
		const json &j = *m_pJ;

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

		jKv(j, "iRCservo", m_iRCservo);
		jKv(j, "pwmServoON", m_pwmServoON);
		jKv(j, "pwmServoOFF", m_pwmServoOFF);

		return true;
	}

	bool _APmav_drive::saveConfig(bool bExport)
	{
		if (!_APmav_move::saveConfig(false))
		{
			return false;
		}

		json &j = *m_pJ;
		j["steer"] = m_steer;
		j["speed"] = m_speed;
		j["pwmM"] = m_pwmM;
		j["pwmD"] = m_pwmD;
		j["iRCsteer"] = m_iRCsteer;
		j["iRCthrottle"] = m_iRCthrottle;
		j["tOutBtn"] = m_tOutBtn;
		j["apModeMove"] = m_apModeMove;
		j["octGridOccu"] = m_octGridOccu;
		j["speedGo"] = m_speedGo;
		j["steerTurn"] = m_steerTurn;
		j["iRCservo"] = m_iRCservo;
		j["pwmServoON"] = m_pwmServoON;
		j["pwmServoOFF"] = m_pwmServoOFF;

		if (!bExport)
		{
			return true;
		}
		return m_pJcfg->saveToFile();
	}

	bool _APmav_drive::link(void)
	{
		IF_F(!this->_APmav_move::link());
		const json &j = *m_pJ;

		string n;

		n = "";
		if (!jKv(j, "_SelectableOctGrid", n))
			jKv(j, "_OctreeGrid", n);
		m_pOctGrid = dynamic_cast<_SelectableOctGrid *>(static_cast<BASE *>(m_pM->findModule(n)));
		IF_Le_F(!m_pOctGrid, "_SelectableOctGrid not found: " + n);

		n = "";
		jKv(j, "_GeoFence", n);
		m_pGfence = (_GeoFence *)(m_pM->findModule(n));
		IF_Le_F(!m_pGfence, "_GeoFence not found: " + n);

		return true;
	}

	bool _APmav_drive::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	bool _APmav_drive::check(void)
	{
		NULL_F(m_pOctGrid);
		NULL_F(m_pGfence);

		return this->_APmav_move::check();
	}

	void _APmav_drive::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPS();

			if (updateCtrl())
			{
				updateDrive();
			}

			ON_PAUSE;
		}
	}

	void _APmav_drive::onPause(void)
	{
		releaseRCoverride();
	}

	bool _APmav_drive::updateCtrl(void)
	{
		IF_F(!check());

		// check btn ctrl
		if (getTns() - m_tLastBtn > m_tOutBtn)
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
			// Vector4d vPos = m_pAP->getGlobalPos();
			// float hdg = m_pAP->getHdg();
			// m_pGfence->setPosHdg(Vector2d(vPos.x(), vPos.y()), hdg);

			// bFenceBreach = m_pGfence->bBreach();
		}

		// stop in Standby mode
		if (m_dMode == apDrive_modeStandby)
		{
			setSteerSpeed(0, 0);
			return true;
		}

		// check AP mode and arming
		// if (m_pAP->getMode() == m_apModeMove)
		// {
		// 	if (!m_pAP->bArmed())
		// 		m_pAP->setArm(true);
		// }
		// else
		// {
		// 	m_pAP->setMode(m_apModeMove);
		// }

		// m_pAP->setMode(m_apModeMove);
		// m_pAP->setArm(true);

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

	void _APmav_drive::setSteerSpeed(float steer, float spd)
	{
		m_steer = steer;
		m_speed = spd;
	}

	void _APmav_drive::updateDrive(void)
	{
		setRCchan(m_iRCservo, m_bServoON ? m_pwmServoON : m_pwmServoOFF, false);
		setRCchan(m_iRCsteer, constrain(m_steer * m_pwmD + m_pwmM, m_pwmM - m_pwmD, m_pwmM + m_pwmD), false);
		setRCchan(m_iRCthrottle, constrain(m_speed * m_pwmD + m_pwmM, m_pwmM - m_pwmD, m_pwmM + m_pwmD), true); // flash cmd to AP mavlink
	}

	void _APmav_drive::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_APmav_move::console(pConsole);

		_Console *pC = (_Console *)pConsole;
		pC->addMsg("steer = " + f2str(m_steer) + ", speed = " + f2str(m_speed));
		pC->addMsg("btnPressed = " + i2str(m_btnPressed));
		pC->addMsg("dMode = " + i2str(m_dMode));
		pC->addMsg("bServoON = " + i2str(m_bServoON));
	}

	void _APmav_drive::console(const json &j, void *pJSONbase)
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
				m_tLastBtn = getTns();
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
		else if (cmd == "setServo")
		{
			const json *servoON = jK(j, "bServoON");
			const bool bSuccess = servoON && (servoON->is_boolean() ||
				(servoON->is_number_integer() && (*servoON == 0 || *servoON == 1)));
			if (bSuccess)
				m_bServoON = servoON->is_boolean() ? servoON->get<bool>() : *servoON == 1;

			NULL_(pJb);
			json jr = json::object();
			jr["cmd"] = "setServo";
			jr["bSuccess"] = bSuccess;
			pJb->sendJson(jr);
		}
	}

}
