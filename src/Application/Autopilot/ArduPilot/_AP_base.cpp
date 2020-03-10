#include "../ArduPilot/_AP_base.h"

namespace kai
{

_AP_base::_AP_base()
{
	m_pMavlink = NULL;
	m_lastHeartbeat = 0;
	m_iHeartbeat = 0;

	m_apType = ardupilot_copter;
	m_apMode = 0;
	m_bApArmed = false;

	m_freqSendHeartbeat = 1;
	m_freqRawSensors = 0;
	m_freqExtStat = 1;
	m_freqRC = 0;
	m_freqPos = 1;
	m_freqExtra1 = 1;

	m_bHomeSet = false;
	m_vHomePos.init(-1.0);
	m_vGlobalPos.init(-1.0);
	m_vLocalPos.init();
	m_vSpeed.init();
	m_apHdg = 0.0;
}

_AP_base::~_AP_base()
{
}

bool _AP_base::init(void* pKiss)
{
	IF_F(!this->_AutopilotBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

	pK->v("apType",(int*)&m_apType);
	pK->v("freqSendHeartbeat",&m_freqSendHeartbeat);
	pK->v("freqRawSensors",&m_freqRawSensors);
	pK->v("freqExtStat",&m_freqExtStat);
	pK->v("freqRC",&m_freqRC);
	pK->v("freqPos",&m_freqPos);
	pK->v("freqExtra1",&m_freqExtra1);

	if(m_freqSendHeartbeat > 0)
		m_freqSendHeartbeat = USEC_1SEC / m_freqSendHeartbeat;
	else
		m_freqSendHeartbeat = 0;

	m_lastHeartbeat = 0;
	m_iHeartbeat = 0;

	string iName;
	iName = "";
	pK->v("_Mavlink", &iName);
	m_pMavlink = (_Mavlink*) (pK->root()->getChildInst(iName));
	NULL_F(m_pMavlink);

	return true;
}

bool _AP_base::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		LOG(ERROR) << "Return code: "<< retCode;
		m_bThreadON = false;
		return false;
	}

	return true;
}

int _AP_base::check(void)
{
	NULL__(m_pMavlink,-1);

	return this->_AutopilotBase::check();
}

void _AP_base::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		this->_AutopilotBase::update();
		updateBase();

		this->autoFPSto();
	}
}

void _AP_base::updateBase(void)
{
	IF_(check()<0);

	//update Ardupilot
	m_apMode = m_pMavlink->m_mavMsg.m_heartbeat.custom_mode;
	m_bApArmed = m_pMavlink->m_mavMsg.m_heartbeat.base_mode & 0b10000000;

	//get home position
	if(m_tStamp - m_pMavlink->m_mavMsg.m_tStamps.m_home_position > USEC_1SEC)
	{
		m_pMavlink->clGetHomePosition();
	}
	else
	{
		m_vHomePos.x = ((double)(m_pMavlink->m_mavMsg.m_home_position.latitude)) * 1e-7;
		m_vHomePos.y = ((double)(m_pMavlink->m_mavMsg.m_home_position.longitude)) * 1e-7;
		m_vHomePos.z = ((double)(m_pMavlink->m_mavMsg.m_home_position.altitude)) * 1e-3;
		m_bHomeSet = true;
	}

	//get position
	if(m_pMavlink->m_mavMsg.m_tStamps.m_global_position_int > 0)
	{
		m_vGlobalPos.x = ((double)(m_pMavlink->m_mavMsg.m_global_position_int.lat)) * 1e-7;
		m_vGlobalPos.y = ((double)(m_pMavlink->m_mavMsg.m_global_position_int.lon)) * 1e-7;
		m_vGlobalPos.z = ((double)(m_pMavlink->m_mavMsg.m_global_position_int.alt)) * 1e-3;
		m_vGlobalPos.w = ((double)(m_pMavlink->m_mavMsg.m_global_position_int.relative_alt)) * 1e-3;
		m_apHdg = ((double)(m_pMavlink->m_mavMsg.m_global_position_int.hdg)) * 1e-2;
	}

	m_vLocalPos.x = m_pMavlink->m_mavMsg.m_local_position_ned.x;
	m_vLocalPos.y = m_pMavlink->m_mavMsg.m_local_position_ned.y;
	m_vLocalPos.z = m_pMavlink->m_mavMsg.m_local_position_ned.z;

	m_vSpeed.x = m_pMavlink->m_mavMsg.m_local_position_ned.vx;
	m_vSpeed.y = m_pMavlink->m_mavMsg.m_local_position_ned.vy;
	m_vSpeed.z = m_pMavlink->m_mavMsg.m_local_position_ned.vz;

	//Attitude
	if(m_pMavlink->m_mavMsg.m_tStamps.m_attitude > 0)
	{
		m_vAtti.x = m_pMavlink->m_mavMsg.m_attitude.yaw;
		m_vAtti.y = m_pMavlink->m_mavMsg.m_attitude.pitch;
		m_vAtti.z = m_pMavlink->m_mavMsg.m_attitude.roll;
	}

	//Send Heartbeat
	if(m_freqSendHeartbeat > 0 && m_tStamp - m_lastHeartbeat >= m_freqSendHeartbeat)
	{
		m_pMavlink->heartbeat();
		m_lastHeartbeat = m_tStamp;
	}

	//request updates from Mavlink
	if(m_freqRawSensors > 0 && m_tStamp - m_pMavlink->m_mavMsg.m_tStamps.m_raw_imu > USEC_1SEC)
		m_pMavlink->requestDataStream(MAV_DATA_STREAM_RAW_SENSORS, m_freqRawSensors);

	if(m_freqExtStat > 0 && m_tStamp - m_pMavlink->m_mavMsg.m_tStamps.m_mission_current > USEC_1SEC)
		m_pMavlink->requestDataStream(MAV_DATA_STREAM_EXTENDED_STATUS, m_freqExtStat);

	if(m_freqRC > 0 && m_tStamp - m_pMavlink->m_mavMsg.m_tStamps.m_rc_channels_raw > USEC_1SEC)
		m_pMavlink->requestDataStream(MAV_DATA_STREAM_RC_CHANNELS, m_freqRC);

	if(m_freqPos > 0 && m_tStamp - m_pMavlink->m_mavMsg.m_tStamps.m_global_position_int > USEC_1SEC)
		m_pMavlink->requestDataStream(MAV_DATA_STREAM_POSITION, m_freqPos);

	if(m_freqExtra1 > 0 && m_tStamp - m_pMavlink->m_mavMsg.m_tStamps.m_attitude > USEC_1SEC)
		m_pMavlink->requestDataStream(MAV_DATA_STREAM_EXTRA1, m_freqExtra1);

}

void _AP_base::setApMode(uint32_t iMode)
{
	IF_(check()<0);

	mavlink_set_mode_t D;
	D.custom_mode = iMode;
	m_pMavlink->setMode(D);
}

uint32_t _AP_base::getApMode(void)
{
	return m_apMode;
}

string _AP_base::getApModeName(void)
{
	if(m_apMode >= AP_N_CUSTOM_MODE)return "?";

	return AP_CUSTOM_MODE_NAME[m_apType][m_apMode];
}

void _AP_base::setApArm(bool bArm)
{
	IF_(check()<0);

	m_pMavlink->clComponentArmDisarm(bArm);
}

bool _AP_base::bApArmed(void)
{
	return m_bApArmed;
}

vDouble3 _AP_base::getHomePos(void)
{
	if(!m_bHomeSet)
	{
		vDouble3 vH;
		vH.init(-1.0);
		return vH;
	}

	return m_vHomePos;
}

vDouble4 _AP_base::getGlobalPos(void)
{
	return m_vGlobalPos;
}

float _AP_base::getApHdg(void)
{
	return m_apHdg;
}

void _AP_base::setMount(AP_MOUNT& m)
{
	IF_(check()<0);

	m_pMavlink->mountControl(m.m_control);
	m_pMavlink->mountConfigure(m.m_config);

	mavlink_param_set_t D;
	D.param_type = MAV_PARAM_TYPE_INT8;
	string id;

	D.param_value = m.m_config.stab_pitch;
	id = "MNT_STAB_TILT";
	strcpy(D.param_id, id.c_str());
	m_pMavlink->param_set(D);

	D.param_value = m.m_config.stab_roll;
	id = "MNT_STAB_ROLL";
	strcpy(D.param_id,id.c_str());
	m_pMavlink->param_set(D);
}

vFloat3 _AP_base::getApSpeed(void)
{
	return m_vSpeed;
}

vFloat3 _AP_base::getApAttitude(void)
{
	return m_vAtti;
}

void _AP_base::draw(void)
{
	this->_AutopilotBase::draw();

	addMsg("State-----------------------------",1);
	if(m_bApArmed)
		addMsg("ARMED",1);
	else
		addMsg("DISARMED",1);

	addMsg("Mode------------------------------",1);
	addMsg("apMode = " + i2str(m_apMode) + ": " + getApModeName(),1);

	addMsg("Attitude--------------------------",1);
	addMsg("y=" + f2str(m_vAtti.x) +
			", p=" + f2str(m_vAtti.y) +
			", r=" + f2str(m_vAtti.z) +
			", hdg=" + f2str(m_apHdg,1));

	addMsg("Global Pos-----------------------",1);
	addMsg("lat=" + lf2str(m_vGlobalPos.x, 7) + ", lon=" + lf2str(m_vGlobalPos.y, 7),1);
	addMsg("alt=" + lf2str(m_vGlobalPos.z, 2) + ", relAlt=" + lf2str(m_vGlobalPos.w, 2),1);

	addMsg("Local Pos-------------------------",1);
	addMsg("x=" + f2str(m_pMavlink->m_mavMsg.m_local_position_ned.x)+
				 ", y=" + f2str(m_pMavlink->m_mavMsg.m_local_position_ned.y)+
				 ", z=" + f2str(m_pMavlink->m_mavMsg.m_local_position_ned.z),1);

	addMsg("Home Pos--------------------------",1);
	addMsg("lat=" + f2str(m_vHomePos.x, 7)
			+ ", lon=" + f2str(m_vHomePos.y, 7)
			+ ", alt=" + f2str(m_vHomePos.z, 7),1);

	addMsg("Speed-----------------------------",1);
	addMsg("vx=" + f2str(m_pMavlink->m_mavMsg.m_local_position_ned.vx)+
				 ", vy=" + f2str(m_pMavlink->m_mavMsg.m_local_position_ned.vy)+
				 ", vz=" + f2str(m_pMavlink->m_mavMsg.m_local_position_ned.vz),1);

	if(m_freqRawSensors > 0)
	{
		addMsg("Sensor----------------------------",1);
		addMsg("xAcc=" + i2str((int32_t)m_pMavlink->m_mavMsg.m_raw_imu.xacc)
					 + ", yAcc=" + i2str((int32_t)m_pMavlink->m_mavMsg.m_raw_imu.yacc)
					 + ", zAcc=" + i2str((int32_t)m_pMavlink->m_mavMsg.m_raw_imu.zacc),1);

		addMsg("xGyro=" + i2str((int32_t)m_pMavlink->m_mavMsg.m_raw_imu.xgyro)
					 + ", yGyro=" + i2str((int32_t)m_pMavlink->m_mavMsg.m_raw_imu.ygyro)
					 + ", zGyro=" + i2str((int32_t)m_pMavlink->m_mavMsg.m_raw_imu.zgyro),1);

		addMsg("xMag=" + i2str((int32_t)m_pMavlink->m_mavMsg.m_raw_imu.xmag)
					 + ", yMag=" + i2str((int32_t)m_pMavlink->m_mavMsg.m_raw_imu.ymag)
					 + ", zMag=" + i2str((int32_t)m_pMavlink->m_mavMsg.m_raw_imu.zmag),1);
	}

}

}
