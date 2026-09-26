#include "_APmav_base.h"

namespace kai
{

	_APmav_base::_APmav_base()
	{
		m_ieSendHB.init(NSEC_SEC);
		m_ieSendMsgInt.init(NSEC_SEC);
	}

	_APmav_base::~_APmav_base()
	{
	}

	bool _APmav_base::loadConfig(void)
	{
		IF_F(!this->_AutopilotBase::loadConfig());
		const json &j = *m_pJ;

		float t;
		if (jKv(j, "ieSendHB", t))
			m_ieSendHB.init(t * NSEC_SEC);

		if (jKv(j, "ieSendMsgInt", t))
			m_ieSendMsgInt.init(t * NSEC_SEC);

		return true;
	}

	bool _APmav_base::link(void)
	{
		IF_F(!this->_AutopilotBase::link());
		const json &j = *m_pJ;

		string n = "";
		jKv(j, "_Mavlink", n);
		m_pMav = (_Mavlink *)(m_pM->findModule(n));
		NULL_F(m_pMav);

		const json *pJm = jK(j, "mavMsgInt");
		IF__(!pJm || !pJm->is_object(), true);
		const json &jm = *pJm;

		for (auto it = jm.begin(); it != jm.end(); it++)
		{
			const json &Ji = it.value();
			IF_CONT(!Ji.is_object());

			int id = 0;
			jKv(Ji, "id", id);
			float tInt = 1;
			jKv(Ji, "tInt", tInt);

			if (!m_pMav->setMsgInterval(id, tInt * NSEC_SEC))
			{
				LOG_E("Interval msg id = " + i2str(id) + " not found");
			}
		}

		return true;
	}

	bool _APmav_base::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	bool _APmav_base::check(void)
	{
		NULL_F(m_pMav);

		return this->_AutopilotBase::check();
	}

	void _APmav_base::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPS();

			updateApMavRecv();

			updateApMavSend();
		}
	}

	void _APmav_base::updateApMavRecv(void)
	{
		IF_(!check());

		uint64_t tNow = getTns();

		if (m_pMav->m_heartbeat.bValid())
		{
			m_customModeFC = m_pMav->m_heartbeat.m_msg.custom_mode;
			// update m_modeFC in inherit class according to vehicle type

			m_armFC = (m_pMav->m_heartbeat.m_msg.base_mode & 0b10000000) ? apArm_arm : apArm_disarm;
		}

		if (m_pMav->m_attitude.bValid())
		{
			m_vAngle.x() = m_pMav->m_attitude.m_msg.roll;
			m_vAngle.y() = m_pMav->m_attitude.m_msg.pitch;
			m_vAngle.z() = m_pMav->m_attitude.m_msg.yaw;
		}

		if (m_pMav->m_globalPositionINT.bValid())
		{
			m_vPos.x() = ((double)(m_pMav->m_globalPositionINT.m_msg.lat)) * 1e-7;
			m_vPos.y() = ((double)(m_pMav->m_globalPositionINT.m_msg.lon)) * 1e-7;
			m_vPos.z() = ((double)(m_pMav->m_globalPositionINT.m_msg.alt)) * 1e-3;

			m_rAlt = ((float)(m_pMav->m_globalPositionINT.m_msg.relative_alt)) * 1e-3;
			//			m_vAngle.z() = ((float)(m_pMav->m_globalPositionINT.m_msg.hdg)) * 1e-2;
		}

		if (m_pMav->m_localPositionNED.bValid())
		{
			m_vVelocity.x() = m_pMav->m_localPositionNED.m_msg.vx;
			m_vVelocity.y() = m_pMav->m_localPositionNED.m_msg.vy;
			m_vVelocity.z() = m_pMav->m_localPositionNED.m_msg.vz;

			// m_vLocalPos.x() = m_pMav->m_localPositionNED.m_msg.x;
			// m_vLocalPos.y() = m_pMav->m_localPositionNED.m_msg.y;
			// m_vLocalPos.z() = m_pMav->m_localPositionNED.m_msg.z;
		}

		if (m_pMav->m_homePosition.bValid())
		{
			m_vHomePos.x() = ((double)(m_pMav->m_homePosition.m_msg.latitude)) * 1e-7;
			m_vHomePos.y() = ((double)(m_pMav->m_homePosition.m_msg.longitude)) * 1e-7;
			m_vHomePos.z() = ((double)(m_pMav->m_homePosition.m_msg.altitude)) * 1e-3;
		}
		else
		{
			//	m_pMav->clGetHomePosition();
		}



		// Battery status
		if (m_pMav->m_batteryStatus.bValid())
		{
			m_battery = (float)(m_pMav->m_batteryStatus.m_msg.battery_remaining) * 0.01;
		}

		// GPS raw
		if (m_pMav->m_gpsRawINT.bValid())
		{
			m_gpsFixType = (int)m_pMav->m_gpsRawINT.m_msg.fix_type;
			m_gpsHacc = m_pMav->m_gpsRawINT.m_msg.h_acc;
		}
	}

	void _APmav_base::updateApMavSend(void)
	{
		IF_(!check());

		if (m_armFC != m_arm)
		{
			m_pMav->clComponentArmDisarm(m_arm == apArm_arm);
		}

		if (m_customModeFC != m_customMode)
		{
			mavlink_set_mode_t D;
			D.custom_mode = m_customMode;
			m_pMav->setMode(D);
		}

		uint64_t tNow = getTns();

		// Send Heartbeat
		if (m_ieSendHB.update(tNow))
		{
			m_pMav->heartbeat();
		}

		if (m_ieSendMsgInt.update(tNow))
		{
			m_pMav->sendSetMsgInterval();
		}
	}

	AP_MODE _APmav_base::getMode(void)
	{
		return m_modeFC;
	}

	AP_ARM _APmav_base::getArm(void)
	{
		return m_armFC;
	}

	void _APmav_base::setCustomMode(int32_t m)
	{
		m_customMode = m;
	}

	int32_t _APmav_base::getCustomMode(void)
	{
		return m_customModeFC;
	}

	void _APmav_base::setMount(AP_MOUNT &m)
	{
		IF_(!check());

		m_pMav->mountControl(m.m_control);
		m_pMav->mountConfigure(m.m_config);

		mavlink_param_set_t D;
		D.param_type = MAV_PARAM_TYPE_INT8;
		string id;

		D.param_value = m.m_config.stab_pitch;
		id = "MNT_STAB_TILT";
		strcpy(D.param_id, id.c_str());
		m_pMav->paramSet(D);

		D.param_value = m.m_config.stab_roll;
		id = "MNT_STAB_ROLL";
		strcpy(D.param_id, id.c_str());
		m_pMav->paramSet(D);
	}

	int _APmav_base::getGPSfixType(void)
	{
		return m_gpsFixType;
	}

	int _APmav_base::getGPShacc(void)
	{
		return m_gpsHacc;
	}

	_Mavlink *_APmav_base::getMavlink(void)
	{
		return m_pMav;
	}

	void _APmav_base::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_AutopilotBase::console(pConsole);

		_Console *pC = (_Console *)pConsole;

		pC->addMsg("-Local Pos-", 1);
		pC->addMsg("\tx=\t" + f2str(m_pMav->m_localPositionNED.m_msg.x) +
					   "\ty=\t" + f2str(m_pMav->m_localPositionNED.m_msg.y) +
					   "\tz=\t" + f2str(m_pMav->m_localPositionNED.m_msg.z),
				   1);

		pC->addMsg("-System-", 1);
		pC->addMsg("\tstatus=\t" + i2str(m_pMav->m_heartbeat.m_msg.system_status));

		pC->addMsg("-Sensor-", 1);
		pC->addMsg("\txAcc=\t" + i2str((int32_t)m_pMav->m_rawIMU.m_msg.xacc) + "\tyAcc=\t" + i2str((int32_t)m_pMav->m_rawIMU.m_msg.yacc) + "\tzAcc=\t" + i2str((int32_t)m_pMav->m_rawIMU.m_msg.zacc), 1);
		pC->addMsg("\txGyro=\t" + i2str((int32_t)m_pMav->m_rawIMU.m_msg.xgyro) + "\tyGyro=\t" + i2str((int32_t)m_pMav->m_rawIMU.m_msg.ygyro) + "\tzGyro=\t" + i2str((int32_t)m_pMav->m_rawIMU.m_msg.zgyro), 1);
		pC->addMsg("\txMag=\t" + i2str((int32_t)m_pMav->m_rawIMU.m_msg.xmag) + "\tyMag=\t" + i2str((int32_t)m_pMav->m_rawIMU.m_msg.ymag) + "\tzMag=\t" + i2str((int32_t)m_pMav->m_rawIMU.m_msg.zmag), 1);
	}

	void _APmav_base::console(const json &j, void *pJSONbase)
	{
		_JSONbase *pJb = (_JSONbase *)pJSONbase;
		string cmd;
		IF_(!jKv(j, "cmd", cmd));

		if (cmd == "setArm")
		{
			bool bArm = false;
			jKv(j, "bArm", bArm);

			NULL_(pJb);
			json jr = json::object();
			jr["cmd"] = "setArm";
			jr["bSuccess"] = true;
			pJb->sendJson(jr);
		}
	}

}
