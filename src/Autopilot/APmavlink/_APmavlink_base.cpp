#include "_APmavlink_base.h"

namespace kai
{

	_APmavlink_base::_APmavlink_base()
	{
		m_vHomePos.setZero();
		m_vGlobalPos.setZero();
		m_vLocalPos.setZero();
		m_vSpeed.setZero();
		m_vAtti.setZero();

		m_ieSendHB.init(USEC_1SEC);
		m_ieSendMsgInt.init(USEC_1SEC);

		m_wrApMode.init(-1, -1);
		m_wrbArm.init(false, false);
	}

	_APmavlink_base::~_APmavlink_base()
	{
	}

	bool _APmavlink_base::init(const json &j)
	{
		IF_F(!this->_ModuleBase::init(j));

		jKv(j, "apType", m_apType);
		jKv(j, "bSyncMode", m_bSyncMode);

		float t;

		if (jKv(j, "ieSendHB", t))
			m_ieSendHB.init(t * SEC_2_USEC);

		if (jKv(j, "ieSendMsgInt", t))
			m_ieSendMsgInt.init(t * SEC_2_USEC);

		return true;
	}

	bool _APmavlink_base::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_ModuleBase::link(j, pM));

		string n = "";
		jKv(j, "_Mavlink", n);
		m_pMav = (_Mavlink *)(pM->findModule(n));
		NULL_F(m_pMav);

		const json &jm = jK(j, "mavMsgInt");
		IF__(!jm.is_object(), true);

		for (auto it = jm.begin(); it != jm.end(); it++)
		{
			const json &Ji = it.value();
			IF_CONT(!Ji.is_object());

			int id = 0;
			jKv(Ji, "id", id);
			float tInt = 1;
			jKv(Ji, "tInt", tInt);

			if (!m_pMav->setMsgInterval(id, tInt * SEC_2_USEC))
			{
				LOG_E("Interval msg id = " + i2str(id) + " not found");
			}
		}

		return true;
	}

	bool _APmavlink_base::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	bool _APmavlink_base::check(void)
	{
		NULL_F(m_pMav);

		return this->_ModuleBase::check();
	}

	void _APmavlink_base::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPS();

			updateBase();

			if (m_bSyncMode)
				updateModeSync();
		}
	}

	void _APmavlink_base::updateBase(void)
	{
		IF_(!check());

		uint64_t tNow = m_pT->getTfrom();

		// update Ardupilot
		if (m_pMav->m_heartbeat.bReceiving())
		{
			m_wrApMode.read(m_pMav->m_heartbeat.m_msg.custom_mode);
			m_wrbArm.read(m_pMav->m_heartbeat.m_msg.base_mode & 0b10000000);
		}

		// Attitude
		//		if (m_pMav->m_attitude.bReceiving())
		//		{
		m_vAtti.x() = m_pMav->m_attitude.m_msg.yaw;
		m_vAtti.y() = m_pMav->m_attitude.m_msg.pitch;
		m_vAtti.z() = m_pMav->m_attitude.m_msg.roll;
		//		}

		// TODO:
		//  get home position
		if (!m_pMav->m_homePosition.bReceiving())
		{
			//			m_pMav->clGetHomePosition();
		}
		else
		{
			m_vHomePos.x() = ((double)(m_pMav->m_homePosition.m_msg.latitude)) * 1e-7;
			m_vHomePos.y() = ((double)(m_pMav->m_homePosition.m_msg.longitude)) * 1e-7;
			m_vHomePos.z() = ((double)(m_pMav->m_homePosition.m_msg.altitude)) * 1e-3;
			m_bHomeSet = true;
		}

		// get position
		if (m_pMav->m_globalPositionINT.bReceiving())
		{
			m_vGlobalPos.x() = ((double)(m_pMav->m_globalPositionINT.m_msg.lat)) * 1e-7;
			m_vGlobalPos.y() = ((double)(m_pMav->m_globalPositionINT.m_msg.lon)) * 1e-7;
			m_vGlobalPos.z() = ((double)(m_pMav->m_globalPositionINT.m_msg.alt)) * 1e-3;
			m_vGlobalPos.w() = ((double)(m_pMav->m_globalPositionINT.m_msg.relative_alt)) * 1e-3;
			m_apHdg = ((float)(m_pMav->m_globalPositionINT.m_msg.hdg)) * 1e-2;
		}

		if (m_pMav->m_localPositionNED.bReceiving())
		{
			m_vLocalPos.x() = m_pMav->m_localPositionNED.m_msg.x;
			m_vLocalPos.y() = m_pMav->m_localPositionNED.m_msg.y;
			m_vLocalPos.z() = m_pMav->m_localPositionNED.m_msg.z;
			m_vSpeed.x() = m_pMav->m_localPositionNED.m_msg.vx;
			m_vSpeed.y() = m_pMav->m_localPositionNED.m_msg.vy;
			m_vSpeed.z() = m_pMav->m_localPositionNED.m_msg.vz;
		}

		// Battery status
		if (m_pMav->m_batteryStatus.bReceiving())
		{
			m_battery = (float)(m_pMav->m_batteryStatus.m_msg.battery_remaining) * 0.01;
		}

		// GPS raw
		if (m_pMav->m_gpsRawINT.bReceiving())
		{
			m_gpsFixType = (int)m_pMav->m_gpsRawINT.m_msg.fix_type;
			m_gpsHacc = m_pMav->m_gpsRawINT.m_msg.h_acc;
		}

		// Send Heartbeat
		if (m_ieSendHB.updateT(tNow))
		{
			m_pMav->heartbeat();
		}

		if (m_ieSendMsgInt.updateT(tNow))
		{
			m_pMav->sendSetMsgInterval();
		}
	}

	void _APmavlink_base::updateModeSync(void)
	{
		IF_(!check());

		if (m_wrApMode.bW())
		{
			m_pMav->clComponentArmDisarm(m_wrbArm.getWrite());
		}

		if (m_wrbArm.bW())
		{
			mavlink_set_mode_t D;
			D.custom_mode = m_wrbArm.getWrite();
			m_pMav->setMode(D);
		}
	}

	void _APmavlink_base::setMode(uint32_t iMode)
	{
		IF_(!check());

		m_wrApMode.write(iMode);

		mavlink_set_mode_t D;
		D.custom_mode = iMode;
		m_pMav->setMode(D);
	}

	int _APmavlink_base::getMode(void)
	{
		return m_wrApMode.getRead();
	}

	string _APmavlink_base::getModeName(void)
	{
		uint32_t iModeR = m_wrApMode.getRead();
		if (iModeR >= AP_N_CUSTOM_MODE)
			return "?";
		if (iModeR < 0)
			return "?";

		if (m_apType == ardupilot_copter)
			return AP_COPTER_CUSTOM_MODE_NAME[iModeR];
		else if (m_apType == ardupilot_rover)
			return AP_ROVER_CUSTOM_MODE_NAME[iModeR];

		return "?";
	}

	void _APmavlink_base::setArm(bool bArm)
	{
		IF_(!check());

		m_wrbArm.write(bArm);
		m_pMav->clComponentArmDisarm(bArm);
	}

	bool _APmavlink_base::bArmed(void)
	{
		return m_wrbArm.getRead();
	}

	void _APmavlink_base::takeOff(float alt)
	{
		IF_(!check());

		m_pMav->clNavTakeoff(alt);
	}

	void _APmavlink_base::setMount(AP_MOUNT &m)
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

	int _APmavlink_base::getGPSfixType(void)
	{
		return m_gpsFixType;
	}

	int _APmavlink_base::getGPShacc(void)
	{
		return m_gpsHacc;
	}

	Vector3d _APmavlink_base::getHomePos(void)
	{
		if (!m_bHomeSet)
		{
			Vector3d vH = Vector3d::Zero();
			vH.setConstant(-1.0);
			return vH;
		}

		return m_vHomePos;
	}

	Vector4d _APmavlink_base::getGlobalPos(void)
	{
		return m_vGlobalPos;
	}

	float _APmavlink_base::getHdg(void)
	{
		return m_apHdg;
	}

	Vector3f _APmavlink_base::getSpeed(void)
	{
		return m_vSpeed;
	}

	Vector3f _APmavlink_base::getAttitude(void)
	{
		return m_vAtti;
	}

	float _APmavlink_base::getBattery(void)
	{
		return m_battery;
	}

	int _APmavlink_base::getWPseq(void)
	{
		IF__(!m_pMav, -1);

		return m_pMav->m_missionCurrent.m_msg.seq;
	}

	int _APmavlink_base::getWPtotal(void)
	{
		IF__(!m_pMav, -1);

		return m_pMav->m_missionCurrent.m_msg.total;
	}

	_Mavlink *_APmavlink_base::getMavlink(void)
	{
		return m_pMav;
	}

	void _APmavlink_base::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);

		_Console *pC = (_Console *)pConsole;
		pC->addMsg("State-----------------------------", 1);
		if (m_wrbArm.getRead())
			pC->addMsg("\tARMED", 1);
		else
			pC->addMsg("\tDISARMED", 1);

		pC->addMsg("Mode------------------------------", 1);
		pC->addMsg("\tapMode=\t" + i2str(getMode()) + "\t" + getModeName(), 1);

		pC->addMsg("Attitude--------------------------", 1);
		pC->addMsg("\ty=\t" + f2str(m_vAtti.x()) +
					   "\tp=\t" + f2str(m_vAtti.y()) +
					   "\tr=\t" + f2str(m_vAtti.z()) +
					   "\thdg=\t" + f2str(m_apHdg),
				   1);

		pC->addMsg("Global Pos-----------------------", 1);
		pC->addMsg("\tGPS fix type=\t" + i2str(m_gpsFixType) + "\tHacc=\t" + i2str(m_gpsHacc), 1);
		pC->addMsg("\tlat=\t" + lf2str(m_vGlobalPos.x(), 7) + "\tlon=\t" + lf2str(m_vGlobalPos.y(), 7), 1);
		pC->addMsg("\talt=\t" + lf2str(m_vGlobalPos.z(), 2) + "\trelAlt=\t" + lf2str(m_vGlobalPos.w(), 2), 1);

		pC->addMsg("Local Pos-------------------------", 1);
		pC->addMsg("\tx=\t" + f2str(m_pMav->m_localPositionNED.m_msg.x) +
					   "\ty=\t" + f2str(m_pMav->m_localPositionNED.m_msg.y) +
					   "\tz=\t" + f2str(m_pMav->m_localPositionNED.m_msg.z),
				   1);

		pC->addMsg("Home Pos--------------------------", 1);
		pC->addMsg("\tlat=\t" + f2str(m_vHomePos.x(), 7) + "\tlon=\t" + f2str(m_vHomePos.y(), 7) + "\talt=\t" + f2str(m_vHomePos.z(), 7), 1);

		pC->addMsg("Speed-----------------------------", 1);
		pC->addMsg("\tvx=\t" + f2str(m_pMav->m_localPositionNED.m_msg.vx) +
					   "\tvy=\t" + f2str(m_pMav->m_localPositionNED.m_msg.vy) +
					   "\tvz=\t" + f2str(m_pMav->m_localPositionNED.m_msg.vz),
				   1);

		pC->addMsg("System-----------------------------", 1);
		pC->addMsg("\tstatus=\t" + i2str(m_pMav->m_heartbeat.m_msg.system_status));

		pC->addMsg("Battery-----------------------------", 1);
		pC->addMsg("\tbatt=\t" + f2str(m_battery));

		pC->addMsg("Sensor----------------------------", 1);
		pC->addMsg("\txAcc=\t" + i2str((int32_t)m_pMav->m_rawIMU.m_msg.xacc) + "\tyAcc=\t" + i2str((int32_t)m_pMav->m_rawIMU.m_msg.yacc) + "\tzAcc=\t" + i2str((int32_t)m_pMav->m_rawIMU.m_msg.zacc), 1);

		pC->addMsg("\txGyro=\t" + i2str((int32_t)m_pMav->m_rawIMU.m_msg.xgyro) + "\tyGyro=\t" + i2str((int32_t)m_pMav->m_rawIMU.m_msg.ygyro) + "\tzGyro=\t" + i2str((int32_t)m_pMav->m_rawIMU.m_msg.zgyro), 1);

		pC->addMsg("\txMag=\t" + i2str((int32_t)m_pMav->m_rawIMU.m_msg.xmag) + "\tyMag=\t" + i2str((int32_t)m_pMav->m_rawIMU.m_msg.ymag) + "\tzMag=\t" + i2str((int32_t)m_pMav->m_rawIMU.m_msg.zmag), 1);
	}

    void _APmavlink_base::console(const json &j, void *pJSONbase)
    {
        _JSONbase *pJb = (_JSONbase *)pJSONbase;
        string cmd;
        IF_(!jKv(j, "cmd", cmd));

        if (cmd == "setArm")
        {
            bool bArm = false;
            jKv(j, "bArm", bArm);

			setArm(bArm);

            NULL_(pJb);
            json jr = json::object();
            jr["cmd"] = "setArm";
            jr["bSuccess"] = true;
            pJb->sendJson(jr);
        }
    }

}
