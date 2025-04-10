#include "_APmavlink_base.h"

namespace kai
{

	_APmavlink_base::_APmavlink_base()
	{
		m_pMav = nullptr;
		m_apType = ardupilot_copter;

		m_bHomeSet = false;
		m_vHomePos.set(0.0);
		m_vGlobalPos.set(0.0);
		m_vLocalPos.clear();
		m_vSpeed.clear();
		m_vAtti.clear();
		m_apHdg = 0.0;
		m_battery = 0.0;
		m_gpsFixType = -1;
		m_gpsHacc = INT32_MAX;

		m_ieSendHB.init(USEC_1SEC);
		m_ieSendMsgInt.init(USEC_1SEC);

		m_bSyncMode = false;
		m_wrApMode.init(-1, -1);
		m_wrbArm.init(false, false);
	}

	_APmavlink_base::~_APmavlink_base()
	{
	}

	int _APmavlink_base::init(void *pKiss)
	{
		CHECK_(this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("apType", (int *)&m_apType);
		pK->v("bSyncMode", &m_bSyncMode);

		float t;

		if (pK->v("ieSendHB", &t))
			m_ieSendHB.init(t * SEC_2_USEC);

		if (pK->v("ieSendMsgInt", &t))
			m_ieSendMsgInt.init(t * SEC_2_USEC);

		return OK_OK;
	}

	int _APmavlink_base::link(void)
	{
		CHECK_(this->_ModuleBase::link());

		Kiss *pK = (Kiss *)m_pKiss;
		string n;

		n = "";
		pK->v("_Mavlink", &n);
		m_pMav = (_Mavlink *)(pK->findModule(n));
		NULL__(m_pMav, OK_ERR_NOT_FOUND);

		Kiss *pM = pK->child("mavMsgInt");
		NULL__(pM, OK_OK);

		int i = 0;
		while (1)
		{
			Kiss *pMI = pM->child(i++);
			if (pMI->empty())
				break;

			int id;
			pMI->v("id", &id);
			float tInt = 1;
			pMI->v("tInt", &tInt);

			if (!m_pMav->setMsgInterval(id, tInt * SEC_2_USEC))
				LOG_E("Interval msg id = " + i2str(id) + " not found");
		}

		return OK_OK;
	}

	int _APmavlink_base::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		return m_pT->start(getUpdate, this);
	}

	int _APmavlink_base::check(void)
	{
		NULL__(m_pMav, OK_ERR_NULLPTR);

		return this->_ModuleBase::check();
	}

	void _APmavlink_base::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			updateBase();

			if (m_bSyncMode)
				updateModeSync();
		}
	}

	void _APmavlink_base::updateBase(void)
	{
		IF_(check() != OK_OK);

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
			m_vAtti.x = m_pMav->m_attitude.m_msg.yaw;
			m_vAtti.y = m_pMav->m_attitude.m_msg.pitch;
			m_vAtti.z = m_pMav->m_attitude.m_msg.roll;
//		}

		// TODO:
		//  get home position
		if (!m_pMav->m_homePosition.bReceiving())
		{
			//			m_pMav->clGetHomePosition();
		}
		else
		{
			m_vHomePos.x = ((double)(m_pMav->m_homePosition.m_msg.latitude)) * 1e-7;
			m_vHomePos.y = ((double)(m_pMav->m_homePosition.m_msg.longitude)) * 1e-7;
			m_vHomePos.z = ((double)(m_pMav->m_homePosition.m_msg.altitude)) * 1e-3;
			m_bHomeSet = true;
		}

		// get position
		if (m_pMav->m_globalPositionINT.bReceiving())
		{
			m_vGlobalPos.x = ((double)(m_pMav->m_globalPositionINT.m_msg.lat)) * 1e-7;
			m_vGlobalPos.y = ((double)(m_pMav->m_globalPositionINT.m_msg.lon)) * 1e-7;
			m_vGlobalPos.z = ((double)(m_pMav->m_globalPositionINT.m_msg.alt)) * 1e-3;
			m_vGlobalPos.w = ((double)(m_pMav->m_globalPositionINT.m_msg.relative_alt)) * 1e-3;
			m_apHdg = ((float)(m_pMav->m_globalPositionINT.m_msg.hdg)) * 1e-2;
		}

		if (m_pMav->m_localPositionNED.bReceiving())
		{
			m_vLocalPos.x = m_pMav->m_localPositionNED.m_msg.x;
			m_vLocalPos.y = m_pMav->m_localPositionNED.m_msg.y;
			m_vLocalPos.z = m_pMav->m_localPositionNED.m_msg.z;
			m_vSpeed.x = m_pMav->m_localPositionNED.m_msg.vx;
			m_vSpeed.y = m_pMav->m_localPositionNED.m_msg.vy;
			m_vSpeed.z = m_pMav->m_localPositionNED.m_msg.vz;
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
		if (m_ieSendHB.update(tNow))
		{
			m_pMav->heartbeat();
		}

		if (m_ieSendMsgInt.update(tNow))
		{
			m_pMav->sendSetMsgInterval();
		}
	}

	void _APmavlink_base::updateModeSync(void)
	{
		IF_(check() != OK_OK);

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
		IF_(check() != OK_OK);

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
		IF_(check() != OK_OK);

		m_wrbArm.write(bArm);
		m_pMav->clComponentArmDisarm(bArm);
	}

	bool _APmavlink_base::bArmed(void)
	{
		return m_wrbArm.getRead();
	}

	void _APmavlink_base::takeOff(float alt)
	{
		IF_(check() != OK_OK);

		m_pMav->clNavTakeoff(alt);
	}

	void _APmavlink_base::setMount(AP_MOUNT &m)
	{
		IF_(check() != OK_OK);

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

	vDouble3 _APmavlink_base::getHomePos(void)
	{
		if (!m_bHomeSet)
		{
			vDouble3 vH;
			vH.set(-1.0);
			return vH;
		}

		return m_vHomePos;
	}

	vDouble4 _APmavlink_base::getGlobalPos(void)
	{
		return m_vGlobalPos;
	}

	float _APmavlink_base::getHdg(void)
	{
		return m_apHdg;
	}

	vFloat3 _APmavlink_base::getSpeed(void)
	{
		return m_vSpeed;
	}

	vFloat3 _APmavlink_base::getAttitude(void)
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

	_Mavlink* _APmavlink_base::getMavlink(void)
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
			pC->addMsg("ARMED", 1);
		else
			pC->addMsg("DISARMED", 1);

		pC->addMsg("Mode------------------------------", 1);
		pC->addMsg("apMode = " + i2str(getMode()) + ": " + getModeName(), 1);

		pC->addMsg("Attitude--------------------------", 1);
		pC->addMsg("y=" + f2str(m_vAtti.x) +
					   ", p=" + f2str(m_vAtti.y) +
					   ", r=" + f2str(m_vAtti.z) +
					   ", hdg=" + f2str(m_apHdg),
				   1);

		pC->addMsg("Global Pos-----------------------", 1);
		pC->addMsg("GPS fix type=" + i2str(m_gpsFixType) + ", Hacc=" + i2str(m_gpsHacc), 1);
		pC->addMsg("lat=" + lf2str(m_vGlobalPos.x, 7) + ", lon=" + lf2str(m_vGlobalPos.y, 7), 1);
		pC->addMsg("alt=" + lf2str(m_vGlobalPos.z, 2) + ", relAlt=" + lf2str(m_vGlobalPos.w, 2), 1);

		pC->addMsg("Local Pos-------------------------", 1);
		pC->addMsg("x=" + f2str(m_pMav->m_localPositionNED.m_msg.x) +
					   ", y=" + f2str(m_pMav->m_localPositionNED.m_msg.y) +
					   ", z=" + f2str(m_pMav->m_localPositionNED.m_msg.z),
				   1);

		pC->addMsg("Home Pos--------------------------", 1);
		pC->addMsg("lat=" + f2str(m_vHomePos.x, 7) + ", lon=" + f2str(m_vHomePos.y, 7) + ", alt=" + f2str(m_vHomePos.z, 7), 1);

		pC->addMsg("Speed-----------------------------", 1);
		pC->addMsg("vx=" + f2str(m_pMav->m_localPositionNED.m_msg.vx) +
					   ", vy=" + f2str(m_pMav->m_localPositionNED.m_msg.vy) +
					   ", vz=" + f2str(m_pMav->m_localPositionNED.m_msg.vz),
				   1);

		pC->addMsg("System-----------------------------", 1);
		pC->addMsg("status=" + i2str(m_pMav->m_heartbeat.m_msg.system_status));

		pC->addMsg("Battery-----------------------------", 1);
		pC->addMsg("batt=" + f2str(m_battery));

		pC->addMsg("Sensor----------------------------", 1);
		pC->addMsg("xAcc=" + i2str((int32_t)m_pMav->m_rawIMU.m_msg.xacc) + ", yAcc=" + i2str((int32_t)m_pMav->m_rawIMU.m_msg.yacc) + ", zAcc=" + i2str((int32_t)m_pMav->m_rawIMU.m_msg.zacc), 1);

		pC->addMsg("xGyro=" + i2str((int32_t)m_pMav->m_rawIMU.m_msg.xgyro) + ", yGyro=" + i2str((int32_t)m_pMav->m_rawIMU.m_msg.ygyro) + ", zGyro=" + i2str((int32_t)m_pMav->m_rawIMU.m_msg.zgyro), 1);

		pC->addMsg("xMag=" + i2str((int32_t)m_pMav->m_rawIMU.m_msg.xmag) + ", yMag=" + i2str((int32_t)m_pMav->m_rawIMU.m_msg.ymag) + ", zMag=" + i2str((int32_t)m_pMav->m_rawIMU.m_msg.zmag), 1);
	}

}
