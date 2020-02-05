#include "../ArduPilot/_AP_base.h"

namespace kai
{

_AP_base::_AP_base()
{
	m_pMavlink = NULL;
	m_lastHeartbeat = 0;
	m_iHeartbeat = 0;
	m_apMode = 0;
	m_lastApMode = 0xffffffff;
	m_bApModeChanged = false;

	m_freqSendHeartbeat = 1;
	m_freqRawSensors = 0;
	m_freqExtStat = 1;
	m_freqRC = 0;
	m_freqPos = 1;
	m_freqExtra1 = 1;

	m_bHomeSet = false;
	m_vHomePos.init();
	m_vGlobalPos.init();
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
	m_apMode = m_pMavlink->m_mavMsg.heartbeat.custom_mode;
	if(m_apMode != m_lastApMode)
	{
		m_bApModeChanged = true;
		m_lastApMode = m_apMode;
	}
	else
	{
		m_bApModeChanged = false;
	}

	//get home position
	if(m_tStamp - m_pMavlink->m_mavMsg.time_stamps.home_position > USEC_1SEC)
	{
		m_pMavlink->clGetHomePosition();
	}
	else
	{
		m_vHomePos.x = ((double)(m_pMavlink->m_mavMsg.home_position.latitude)) * 1e-7;
		m_vHomePos.y = ((double)(m_pMavlink->m_mavMsg.home_position.longitude)) * 1e-7;
		m_vHomePos.z = ((double)(m_pMavlink->m_mavMsg.home_position.altitude)) * 1e-3;
		m_bHomeSet = true;
	}

	//get position
	m_vGlobalPos.x = ((double)(m_pMavlink->m_mavMsg.global_position_int.lat)) * 1e-7;
	m_vGlobalPos.y = ((double)(m_pMavlink->m_mavMsg.global_position_int.lon)) * 1e-7;
	m_vGlobalPos.z = ((double)(m_pMavlink->m_mavMsg.global_position_int.relative_alt)) * 1e-3;
	m_apHdg = ((double)(m_pMavlink->m_mavMsg.global_position_int.hdg)) * 1e-2;

	m_vLocalPos.x = m_pMavlink->m_mavMsg.local_position_ned.x;
	m_vLocalPos.y = m_pMavlink->m_mavMsg.local_position_ned.y;
	m_vLocalPos.z = m_pMavlink->m_mavMsg.local_position_ned.z;

	m_vSpeed.x = m_pMavlink->m_mavMsg.local_position_ned.vx;
	m_vSpeed.y = m_pMavlink->m_mavMsg.local_position_ned.vy;
	m_vSpeed.z = m_pMavlink->m_mavMsg.local_position_ned.vz;

	//Send Heartbeat
	if(m_freqSendHeartbeat > 0 && m_tStamp - m_lastHeartbeat >= m_freqSendHeartbeat)
	{
		m_pMavlink->heartbeat();
		m_lastHeartbeat = m_tStamp;
	}

	//request updates from Mavlink
	if(m_freqRawSensors > 0 && m_tStamp - m_pMavlink->m_mavMsg.time_stamps.raw_imu > USEC_1SEC)
		m_pMavlink->requestDataStream(MAV_DATA_STREAM_RAW_SENSORS, m_freqRawSensors);

	if(m_freqExtStat > 0 && m_tStamp - m_pMavlink->m_mavMsg.time_stamps.mission_current > USEC_1SEC)
		m_pMavlink->requestDataStream(MAV_DATA_STREAM_EXTENDED_STATUS, m_freqExtStat);

	if(m_freqRC > 0 && m_tStamp - m_pMavlink->m_mavMsg.time_stamps.rc_channels_raw > USEC_1SEC)
		m_pMavlink->requestDataStream(MAV_DATA_STREAM_RC_CHANNELS, m_freqRC);

	if(m_freqPos > 0 && m_tStamp - m_pMavlink->m_mavMsg.time_stamps.global_position_int > USEC_1SEC)
		m_pMavlink->requestDataStream(MAV_DATA_STREAM_POSITION, m_freqPos);

	if(m_freqExtra1 > 0 && m_tStamp - m_pMavlink->m_mavMsg.time_stamps.attitude > USEC_1SEC)
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
	if(m_apMode >= AP_N_CUSTOM_MODE)return "UNDEFINED";

	return custom_mode_name[m_apMode];
}

int _AP_base::getApModeByName(const string& name)
{
	for(int i=0; i<AP_N_CUSTOM_MODE; i++)
	{
		IF_CONT(custom_mode_name[i] != name);
		return i;
	}

	return -1;
}

bool _AP_base::bApModeChanged(void)
{
	return m_bApModeChanged;
}

void _AP_base::setApArm(bool bArm)
{
	IF_(check()<0);

	m_pMavlink->clComponentArmDisarm(bArm);
}

bool _AP_base::getApArm(void)
{
	return false; //TODO
}

bool _AP_base::getHomePos(vDouble3* pHome)
{
	NULL_F(pHome);
	IF_F(!m_bHomeSet);

	*pHome = m_vHomePos;
	return true;
}

vDouble3 _AP_base::getGlobalPos(void)
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

void _AP_base::draw(void)
{
	this->_AutopilotBase::draw();

	addMsg("apMode = " + i2str(m_apMode) + ": " + getApModeName(),1);

	addMsg("y=" + f2str(m_pMavlink->m_mavMsg.attitude.yaw) +
			", p=" + f2str(m_pMavlink->m_mavMsg.attitude.pitch) +
			", r=" + f2str(m_pMavlink->m_mavMsg.attitude.roll) +
			", hdg=" + f2str(((float)m_pMavlink->m_mavMsg.global_position_int.hdg)*1e-2),1);

	addMsg("alt=" + f2str(((float)m_pMavlink->m_mavMsg.global_position_int.alt)*1e-3)
		  + ", relAlt=" + f2str(((float)m_pMavlink->m_mavMsg.global_position_int.relative_alt)*1e-3),1);

	addMsg("lat=" + f2str(((float)m_pMavlink->m_mavMsg.global_position_int.lat)*1e-7, 7)
		  + ", lon=" + f2str(((float)m_pMavlink->m_mavMsg.global_position_int.lon)*1e-7, 7),1);

	addMsg("Home: lat=" + f2str(m_vHomePos.x, 7)
				 + ", lon=" + f2str(m_vHomePos.y, 7)
				 + ", alt=" + f2str(m_vHomePos.z, 7)
				 ,1);

	addMsg("x=" + f2str(m_pMavlink->m_mavMsg.local_position_ned.x)+
				 ", y=" + f2str(m_pMavlink->m_mavMsg.local_position_ned.y)+
				 ", z=" + f2str(m_pMavlink->m_mavMsg.local_position_ned.z),1);

	addMsg("vx=" + f2str(m_pMavlink->m_mavMsg.local_position_ned.vx)+
				 ", vy=" + f2str(m_pMavlink->m_mavMsg.local_position_ned.vy)+
				 ", vz=" + f2str(m_pMavlink->m_mavMsg.local_position_ned.vz),1);

	if(m_freqRawSensors > 0)
	{
		addMsg("xAcc=" + i2str((int32_t)m_pMavlink->m_mavMsg.raw_imu.xacc)
					 + ", yAcc=" + i2str((int32_t)m_pMavlink->m_mavMsg.raw_imu.yacc)
					 + ", zAcc=" + i2str((int32_t)m_pMavlink->m_mavMsg.raw_imu.zacc),1);

		addMsg("xGyro=" + i2str((int32_t)m_pMavlink->m_mavMsg.raw_imu.xgyro)
					 + ", yGyro=" + i2str((int32_t)m_pMavlink->m_mavMsg.raw_imu.ygyro)
					 + ", zGyro=" + i2str((int32_t)m_pMavlink->m_mavMsg.raw_imu.zgyro),1);

		addMsg("xMag=" + i2str((int32_t)m_pMavlink->m_mavMsg.raw_imu.xmag)
					 + ", yMag=" + i2str((int32_t)m_pMavlink->m_mavMsg.raw_imu.ymag)
					 + ", zMag=" + i2str((int32_t)m_pMavlink->m_mavMsg.raw_imu.zmag),1);
	}

}

}
