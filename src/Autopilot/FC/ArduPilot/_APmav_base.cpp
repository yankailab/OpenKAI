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

		jKv(j, "myType", m_myType);
		jKv(j, "mySysID", m_mySysID);
		jKv(j, "myComID", m_myComID);
		jKv(j, "targetSysID", m_targetSysID);
		jKv(j, "targetComID", m_targetComID);

		double t;
		if (jKv(j, "ieSendHB", t))
			m_ieSendHB.init(t * NSEC_SEC);

		if (jKv(j, "ieSendMsgInt", t))
			m_ieSendMsgInt.init(t * NSEC_SEC);

		return true;
	}

	bool _APmav_base::saveConfig(bool bExport)
	{
		IF_F(!_AutopilotBase::saveConfig(false));

		json &j = *m_pJ;
		j["myType"] = m_myType;
		j["mySysID"] = m_mySysID;
		j["myComID"] = m_myComID;
		j["targetSysID"] = m_targetSysID;
		j["targetComID"] = m_targetComID;
		j["ieSendHB"] = static_cast<double>(m_ieSendHB.m_tInterval) / NSEC_SEC;
		j["ieSendMsgInt"] = static_cast<double>(m_ieSendMsgInt.m_tInterval) / NSEC_SEC;

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool _APmav_base::link(InstanceMgr *pM)
	{
		IF_F(!this->_AutopilotBase::link(pM));
		const json &j = *m_pJ;

		string n = "";
		jKv(j, "MavlinkStreamIn", n);
		m_pMavStreamIn = dynamic_cast<MavlinkStream *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
		NULL_F(m_pMavStreamIn);

		n.clear();
		jKv(j, "MavlinkStreamOut", n);
		m_pMavStreamOut = dynamic_cast<MavlinkStream *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
		NULL_F(m_pMavStreamOut);

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

			if (!m_pMavStreamIn->setMsgInterval(id, tInt * NSEC_SEC))
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
		NULL_F(m_pMavStreamIn);
		NULL_F(m_pMavStreamOut);

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

		if (auto *pM = m_pMavStreamIn->getMsg<MavHeartbeat>(); pM && pM->bValid())
		{
			const auto &msg = pM->get();
			m_customModeFC = msg.custom_mode;
			// update m_modeFC in inherit class according to vehicle type

			m_armFC = (msg.base_mode & 0b10000000) ? apArm_arm : apArm_disarm;
		}

		if (auto *pM = m_pMavStreamIn->getMsg<MavAttitude>(); pM && pM->bValid())
		{
			const auto &msg = pM->get();
			m_vAngle.x() = msg.roll;
			m_vAngle.y() = msg.pitch;
			m_vAngle.z() = msg.yaw;
		}

		if (auto *pM = m_pMavStreamIn->getMsg<MavGlobalPositionINT>(); pM && pM->bValid())
		{
			const auto &msg = pM->get();
			m_vPos.x() = ((double)(msg.lat)) * 1e-7;
			m_vPos.y() = ((double)(msg.lon)) * 1e-7;
			m_vPos.z() = ((double)(msg.alt)) * 1e-3;

			m_rAlt = ((float)(msg.relative_alt)) * 1e-3;
			//			m_vAngle.z() = ((float)(msg.hdg)) * 1e-2;
		}

		if (auto *pM = m_pMavStreamIn->getMsg<MavLocalPositionNED>(); pM && pM->bValid())
		{
			const auto &msg = pM->get();
			m_vVelocity.x() = msg.vx;
			m_vVelocity.y() = msg.vy;
			m_vVelocity.z() = msg.vz;

			// m_vLocalPos.x() = msg.x;
			// m_vLocalPos.y() = msg.y;
			// m_vLocalPos.z() = msg.z;
		}

		if (auto *pM = m_pMavStreamIn->getMsg<MavHomePosition>(); pM && pM->bValid())
		{
			const auto &msg = pM->get();
			m_vHomePos.x() = ((double)(msg.latitude)) * 1e-7;
			m_vHomePos.y() = ((double)(msg.longitude)) * 1e-7;
			m_vHomePos.z() = ((double)(msg.altitude)) * 1e-3;
		}
		else
		{
			//	m_pMavStreamOut->clGetHomePosition();
		}



		// Battery status
		if (auto *pM = m_pMavStreamIn->getMsg<MavBatteryStatus>(); pM && pM->bValid())
		{
			const auto &msg = pM->get();
			m_battery = (float)(msg.battery_remaining) * 0.01;
		}

		// GPS raw
		if (auto *pM = m_pMavStreamIn->getMsg<MavGpsRawINT>(); pM && pM->bValid())
		{
			const auto &msg = pM->get();
			m_gpsFixType = (int)msg.fix_type;
			m_gpsHacc = msg.h_acc;
		}
	}

	void _APmav_base::updateApMavSend(void)
	{
		IF_(!check());

		if (m_armFC != m_arm)
		{
			m_pMavStreamOut->clComponentArmDisarm(m_arm == apArm_arm);
		}

		if (m_customModeFC != m_customMode)
		{
			mavlink_set_mode_t D{};
			D.base_mode = MAV_MODE_FLAG_CUSTOM_MODE_ENABLED;
			D.custom_mode = m_customMode;
			m_pMavStreamOut->setMode(D);
		}

		uint64_t tNow = getTns();

		// Send Heartbeat
		if (m_ieSendHB.update(tNow))
		{
			mavlink_heartbeat_t heartbeat{};
			heartbeat.type = m_myType;
			heartbeat.autopilot = MAV_AUTOPILOT_INVALID;
			heartbeat.system_status = MAV_STATE_ACTIVE;
			m_pMavStreamOut->heartbeat(heartbeat);
		}

		if (m_ieSendMsgInt.update(tNow))
		{
			m_pMavStreamIn->sendSetMsgInterval(m_pMavStreamOut);
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

		m_pMavStreamOut->mountControl(m.m_control);
		m_pMavStreamOut->mountConfigure(m.m_config);

		mavlink_param_set_t D{};
		D.param_type = MAV_PARAM_TYPE_INT8;
		string id;

		D.param_value = m.m_config.stab_pitch;
		id = "MNT_STAB_TILT";
		strcpy(D.param_id, id.c_str());
		m_pMavStreamOut->paramSet(D);

		D.param_value = m.m_config.stab_roll;
		id = "MNT_STAB_ROLL";
		strcpy(D.param_id, id.c_str());
		m_pMavStreamOut->paramSet(D);
	}

	int _APmav_base::getGPSfixType(void)
	{
		return m_gpsFixType;
	}

	int _APmav_base::getGPShacc(void)
	{
		return m_gpsHacc;
	}

	MavlinkStream *_APmav_base::getMavlinkStreamIn(void)
	{
		return m_pMavStreamIn;
	}

	MavlinkStream *_APmav_base::getMavlinkStreamOut(void)
	{
		return m_pMavStreamOut;
	}

	void _APmav_base::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_AutopilotBase::console(pConsole);
		NULL_(m_pMavStreamIn);

		_Console *pC = (_Console *)pConsole;
		const auto &position = m_pMavStreamIn->getMsg<MavLocalPositionNED>()->get();
		const auto &heartbeat = m_pMavStreamIn->getMsg<MavHeartbeat>()->get();
		const auto &imu = m_pMavStreamIn->getMsg<MavRawIMU>()->get();

		pC->addMsg("-Local Pos-", 1);
		pC->addMsg("\tx=\t" + f2str(position.x) +
					   "\ty=\t" + f2str(position.y) +
					   "\tz=\t" + f2str(position.z),
				   1);

		pC->addMsg("-System-", 1);
		pC->addMsg("\tstatus=\t" + i2str(heartbeat.system_status));

		pC->addMsg("-Sensor-", 1);
		pC->addMsg("\txAcc=\t" + i2str((int32_t)imu.xacc) + "\tyAcc=\t" + i2str((int32_t)imu.yacc) + "\tzAcc=\t" + i2str((int32_t)imu.zacc), 1);
		pC->addMsg("\txGyro=\t" + i2str((int32_t)imu.xgyro) + "\tyGyro=\t" + i2str((int32_t)imu.ygyro) + "\tzGyro=\t" + i2str((int32_t)imu.zgyro), 1);
		pC->addMsg("\txMag=\t" + i2str((int32_t)imu.xmag) + "\tyMag=\t" + i2str((int32_t)imu.ymag) + "\tzMag=\t" + i2str((int32_t)imu.zmag), 1);
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
