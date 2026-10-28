#include "_AProver_LiDAR.h"
#include <cmath>

namespace kai
{
	namespace
	{
		float headingDelta(float target, float heading)
		{
			return std::remainder(target - heading, 360.0f);
		}
	}

	_AProver_LiDAR::_AProver_LiDAR()
	{
	}

	_AProver_LiDAR::~_AProver_LiDAR()
	{
		if (m_pT)
			m_pT->join();
	}

	bool _AProver_LiDAR::loadConfig(void)
	{
		IF_F(!this->_APmav_move::loadConfig());
		std::lock_guard<std::recursive_mutex> lock(m_ctrlMutex);
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
		jKv(j, "bumperTurnDeg", m_bumperTurnDeg);
		jKv(j, "hdgToleranceDeg", m_hdgToleranceDeg);
		jKv(j, "tOutNav", m_tOutNav);

		jKv(j, "iRCservo", m_iRCservo);
		jKv(j, "pwmServoON", m_pwmServoON);
		jKv(j, "pwmServoOFF", m_pwmServoOFF);

		IF_Le_F(!std::isfinite(m_speedGo) || m_speedGo <= 0 || m_speedGo > 1 ||
				!std::isfinite(m_steerTurn) || m_steerTurn <= 0 || m_steerTurn > 1,
				"speedGo and steerTurn must be in (0, 1]");
		IF_Le_F(!std::isfinite(m_bumperTurnDeg) || std::fabs(m_bumperTurnDeg) > 180 ||
				!std::isfinite(m_hdgToleranceDeg) || m_hdgToleranceDeg <= 0 || m_hdgToleranceDeg >= 90,
				"Invalid bumper heading offset or heading tolerance");
		IF_Le_F(!m_tOutBtn || !m_tOutNav || !std::isfinite(m_octGridOccu) || m_octGridOccu < 0,
				"Invalid control timeout or occupancy threshold");
		stopControl(true);
		return true;
	}

	bool _AProver_LiDAR::saveConfig(bool bExport)
	{
		std::lock_guard<std::recursive_mutex> lock(m_ctrlMutex);
		IF_F(!_APmav_move::saveConfig(false));

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
		j["bumperTurnDeg"] = m_bumperTurnDeg;
		j["hdgToleranceDeg"] = m_hdgToleranceDeg;
		j["tOutNav"] = m_tOutNav;
		j["iRCservo"] = m_iRCservo;
		j["pwmServoON"] = m_pwmServoON;
		j["pwmServoOFF"] = m_pwmServoOFF;

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool _AProver_LiDAR::link(InstanceMgr *pM)
	{
		IF_F(!this->_APmav_move::link(pM));
		const json &j = *m_pJ;

		string n;

		n = "";
		if (!jKv(j, "_SelectableOctGrid", n))
			jKv(j, "_OctreeGrid", n);
		m_pOctGrid = dynamic_cast<_SelectableOctGrid *>(static_cast<BASE *>(pM->findModule(n)));
		IF_Le_F(!m_pOctGrid, "_SelectableOctGrid not found: " + n);

		n = "";
		jKv(j, "_GeoFence", n);
		m_pGfence = (_GeoFence *)(pM->findModule(n));
		IF_Le_F(!m_pGfence, "_GeoFence not found: " + n);

		return true;
	}

	bool _AProver_LiDAR::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	void _AProver_LiDAR::stop(void)
	{
		// Finish the worker's neutral/cutter-off output before linked modules are destroyed.
		if (m_pT)
			m_pT->join();
	}

	bool _AProver_LiDAR::check(void)
	{
		NULL_F(m_pOctGrid);
		NULL_F(m_pGfence);

		return this->_APmav_move::check();
	}

	void _AProver_LiDAR::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPS();
			if (!m_pT->bRun())
				break;
			{
				std::lock_guard<std::recursive_mutex> lock(m_ctrlMutex);
				if (updateCtrl())
					updateDrive();
			}
			ON_PAUSE;
		}
		onPause();
	}

	void _AProver_LiDAR::resetAvoidance(void)
	{
		m_bumperState = bumperClear;
		m_bFenceTurning = false;
	}

	void _AProver_LiDAR::stopControl(bool cutterOff)
	{
		m_dMode = apLiDAR_modeStandby;
		m_btnPressed = apLiDAR_btnNone;
		m_bDirectControl = false;
		resetAvoidance();
		setSteerSpeed(0, 0);
		if (cutterOff)
			m_bServoON = false;
	}

	void _AProver_LiDAR::onPause(void)
	{
		std::lock_guard<std::recursive_mutex> lock(m_ctrlMutex);
		stopControl(true);
		// Keep neutral/cutter-off overrides while paused; resume requires a new command.
		if (_APmav_move::check())
			updateDrive();
	}

	bool _AProver_LiDAR::getNavigation(Vector4d &position)
	{
		NULL_F(m_pAP);
		auto *stream = m_pAP->getMavlinkStream();
		NULL_F(stream);
		mavlink_global_position_int_t msg{};
		uint64_t timestamp = 0;
		IF_F(!stream->snapshot<MavGlobalPositionINT>(msg, timestamp));
		IF_F(getTns() - timestamp > m_tOutNav || msg.hdg >= 36000);
		// GLOBAL_POSITION_INT uses centidegrees; UINT16_MAX means unknown heading.
		position = Vector4d(msg.lat * 1e-7, msg.lon * 1e-7, msg.alt * 1e-3, msg.hdg * 1e-2);
		return true;
	}

	bool _AProver_LiDAR::updateCtrl(void)
	{
		std::lock_guard<std::recursive_mutex> lock(m_ctrlMutex);
		if (!check())
		{
			stopControl(true);
			return false;
		}

		updateButton();
		if (m_bEmergencyBtn)
		{
			stopControl(true);
			return true;
		}

		if (m_dMode == apLiDAR_modeManual && getTns() - m_tLastBtn > m_tOutBtn)
			stopControl(false);

		Vector4d position;
		const bool navigationValid = getNavigation(position);
		if (navigationValid)
			m_pGfence->setPosHdg(Vector2d(position.x(), position.y()), position.w());

		if (m_dMode == apLiDAR_modeStandby)
		{
			setSteerSpeed(0, 0);
			return true;
		}
		const auto fence = m_pGfence->getStatus();
		if (m_dMode == apLiDAR_modeAuto && (!navigationValid || !fence.bValid))
		{
			stopControl(true);
			return true;
		}

		// Compare ArduRover custom modes, not the framework's AP_MODE enum.
		if (m_pAP->getCustomMode() != m_apModeMove)
		{
			m_pAP->setCustomMode(m_apModeMove);
			setSteerSpeed(0, 0);
			return true;
		}
		m_pAP->setArm(apArm_arm);
		if (m_pAP->getArm() != apArm_arm)
		{
			setSteerSpeed(0, 0);
			return true;
		}

		if (m_dMode == apLiDAR_modeManual)
		{
			resetAvoidance();
			if (m_btnPressed == apLiDAR_btnForward)
				setSteerSpeed(0, m_speedGo);
			else if (m_btnPressed == apLiDAR_btnBackward)
				setSteerSpeed(0, -m_speedGo);
			else if (m_btnPressed == apLiDAR_btnLeft)
				setSteerSpeed(-m_steerTurn, 0);
			else if (m_btnPressed == apLiDAR_btnRight)
				setSteerSpeed(m_steerTurn, 0);
			else if (m_bDirectControl)
				setSteerSpeed(m_directSteer, m_directSpeed);
			else
				setSteerSpeed(0, 0);
		}
		else if (m_dMode == apLiDAR_modeAuto)
		{
			updateAuto(position.w(), fence);
		}
		return true;
	}

	void _AProver_LiDAR::updateAuto(float heading, const _GeoFence_STATUS &fence)
	{
		// Contact takes priority over prediction: back away until the bumper releases.
		if (m_bBumper)
		{
			m_bumperState = bumperReverse;
			m_bFenceTurning = false;
			setSteerSpeed(0, -m_speedGo);
			return;
		}
		if (m_bumperState == bumperReverse)
		{
			m_bumperTargetHdg = heading + m_bumperTurnDeg;
			m_bumperState = bumperTurn;
		}
		if (m_bumperState == bumperTurn)
		{
			float remaining = headingDelta(m_bumperTargetHdg, heading);
			const float direction = m_bumperTurnDeg < 0 ? -1.0f : 1.0f;
			if (std::fabs(remaining) == 180.0f)
				remaining = 180.0f * direction;
			// Accept passing the target between samples as well as reaching its tolerance.
			if (std::fabs(remaining) > m_hdgToleranceDeg && remaining * direction > 0)
			{
				setSteerSpeed(direction * m_steerTurn, 0);
				return;
			}
			m_bumperState = bumperClear;
		}

		if (fence.bTurnRequired)
		{
			if (!m_bFenceTurning)
			{
				m_fenceTurnDir = fence.turnDeg < 0 ? -1.0f : 1.0f;
				m_bFenceTurning = true;
			}
			setSteerSpeed(m_fenceTurnDir * m_steerTurn, 0);
			return;
		}
		if (m_pOctGrid->bSelectedOccupied(m_octGridOccu))
		{
			// Keep a fence encounter's direction until both hazards clear, avoiding
			// a right obstacle turn that steers back into a left fence turn.
			setSteerSpeed((m_bFenceTurning ? m_fenceTurnDir : 1.0f) * m_steerTurn, 0);
		}
		else
		{
			m_bFenceTurning = false;
			setSteerSpeed(0, m_speedGo);
		}
	}

	void _AProver_LiDAR::setSteerSpeed(float steer, float spd)
	{
		std::lock_guard<std::recursive_mutex> lock(m_ctrlMutex);
		m_steer = std::isfinite(steer) ? constrain(steer, -1.0f, 1.0f) : 0;
		m_speed = std::isfinite(spd) ? constrain(spd, -1.0f, 1.0f) : 0;
	}

	void _AProver_LiDAR::updateButton(void)
	{
		std::lock_guard<std::recursive_mutex> lock(m_ctrlMutex);
		NULL_(m_pAP);
		auto *stream = m_pAP->getMavlinkStream();
		NULL_(stream);
		mavlink_button_change_t msg{};
		uint64_t timestamp = 0;
		if (stream->snapshot<MavButtonChange>(msg, timestamp))
		{
			m_bEmergencyBtn = (msg.state & (1u << 0)) != 0;
			m_bBumper = (msg.state & (1u << 1)) != 0;
		}
	}

	void _AProver_LiDAR::updateDrive(void)
	{
		std::lock_guard<std::recursive_mutex> lock(m_ctrlMutex);
		setRCchan(m_iRCservo, m_bServoON ? m_pwmServoON : m_pwmServoOFF, false);
		setRCchan(m_iRCsteer, constrain(m_steer * m_pwmD + m_pwmM, m_pwmM - m_pwmD, m_pwmM + m_pwmD), false);
		setRCchan(m_iRCthrottle, constrain(m_speed * m_pwmD + m_pwmM, m_pwmM - m_pwmD, m_pwmM + m_pwmD), true); // flash cmd to AP mavlink
	}

	void _AProver_LiDAR::console(void *pConsole)
	{
		std::lock_guard<std::recursive_mutex> lock(m_ctrlMutex);
		NULL_(pConsole);
		this->_APmav_move::console(pConsole);

		_Console *pC = (_Console *)pConsole;
		pC->addMsg("steer = " + f2str(m_steer) + ", speed = " + f2str(m_speed));
		pC->addMsg("btnPressed = " + i2str(m_btnPressed));
		pC->addMsg("dMode = " + i2str(m_dMode));
		pC->addMsg("bServoON = " + i2str(m_bServoON));
		pC->addMsg("emergency = " + i2str(m_bEmergencyBtn) + ", bumper = " + i2str(m_bBumper));
		pC->addMsg("bumperState = " + i2str(m_bumperState));
	}

	void _AProver_LiDAR::console(const json &j, void *pJSONbase)
	{
		std::lock_guard<std::recursive_mutex> lock(m_ctrlMutex);
		_JSONbase *pJb = (_JSONbase *)pJSONbase;
		string cmd;
		IF_(!jKv(j, "cmd", cmd));

		updateButton();
		if (m_bEmergencyBtn)
			stopControl(true);

		bool bSuccess = false;
		string error;
		if (cmd == "setSteerSpeed")
		{
			const json *steer = jK(j, "steer"), *speed = jK(j, "speed");
			bSuccess = !m_bEmergencyBtn && steer && speed && steer->is_number() && speed->is_number();
			if (bSuccess)
			{
				const float s = steer->get<float>(), v = speed->get<float>();
				bSuccess = std::isfinite(s) && std::isfinite(v);
				if (bSuccess)
				{
					resetAvoidance();
					m_dMode = apLiDAR_modeManual;
					m_btnPressed = apLiDAR_btnNone;
					m_bDirectControl = true;
					m_directSteer = s;
					m_directSpeed = v;
					m_tLastBtn = getTns();
					setSteerSpeed(s, v);
				}
			}
		}
		else if (cmd == "ctrlBtn")
		{
			string btn;
			jKv(j, "btn", btn);
			AP_LIDAR_BTN pressed = apLiDAR_btnNone;
			if (btn == "F") pressed = apLiDAR_btnForward;
			else if (btn == "L") pressed = apLiDAR_btnLeft;
			else if (btn == "R") pressed = apLiDAR_btnRight;
			else if (btn == "B") pressed = apLiDAR_btnBackward;
			else if (btn == "S") pressed = apLiDAR_btnStop;

			bSuccess = pressed != apLiDAR_btnNone && (!m_bEmergencyBtn || pressed == apLiDAR_btnStop);
			if (bSuccess)
			{
				if (pressed == apLiDAR_btnStop)
					stopControl(false); // STOP is latched immediately, even between control ticks.
				else
				{
					resetAvoidance();
					m_dMode = apLiDAR_modeManual;
					m_bDirectControl = false;
					m_btnPressed = pressed;
					m_tLastBtn = getTns();
				}
			}
		}
		else if (cmd == "startAuto")
		{
			Vector4d position;
			bSuccess = !m_bEmergencyBtn && m_pGfence && getNavigation(position);
			if (bSuccess)
			{
				m_pGfence->setPosHdg(Vector2d(position.x(), position.y()), position.w());
				bSuccess = m_pGfence->getStatus().bValid;
			}
			if (bSuccess)
			{
				resetAvoidance();
				m_btnPressed = apLiDAR_btnNone;
				m_bDirectControl = false;
				m_dMode = apLiDAR_modeAuto;
			}
			else
			{
				stopControl(true);
				error = "Auto requires a valid polygon fence and fresh position/heading telemetry";
			}
		}
		else if (cmd == "setServo")
		{
			const json *servoON = jK(j, "bServoON");
			bSuccess = servoON && (servoON->is_boolean() ||
				(servoON->is_number_integer() && (*servoON == 0 || *servoON == 1)));
			if (bSuccess)
			{
				const bool on = servoON->is_boolean() ? servoON->get<bool>() : *servoON == 1;
				bSuccess = !on || !m_bEmergencyBtn;
				if (bSuccess)
					m_bServoON = on;
			}
		}
		else
			return;

		if (!bSuccess && m_bEmergencyBtn)
			error = "Emergency button is pressed";
		NULL_(pJb);
		json jr = {{"cmd", cmd}, {"bSuccess", bSuccess}};
		if (!error.empty())
			jr["error"] = error;
		pJb->sendJson(jr);
	}

}
