#include "../ArduPilot/_AP_base.h"

namespace kai
{

_AP_base::_AP_base()
{
	m_pMav = NULL;
	m_lastHeartbeat = 0;
	m_iHeartbeat = 0;

	m_apType = ardupilot_copter;
	m_apMode = 0;
	m_bApArmed = false;
	m_freqSendHeartbeat = 1;

	m_bHomeSet = false;
	m_vHomePos.init(-1.0);
	m_vGlobalPos.init(-1.0);
	m_vLocalPos.init();
	m_vSpeed.init();
	m_vAtti.init();
	m_apHdg = -1.0;
}

_AP_base::~_AP_base()
{
}

bool _AP_base::init(void* pKiss)
{
	IF_F(!this->_MissionBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

	pK->v("apType",(int*)&m_apType);
	pK->v("freqSendHeartbeat",&m_freqSendHeartbeat);

	if(m_freqSendHeartbeat > 0)
		m_freqSendHeartbeat = USEC_1SEC / m_freqSendHeartbeat;
	else
		m_freqSendHeartbeat = 0;

	m_lastHeartbeat = 0;
	m_iHeartbeat = 0;

	string iName;
	iName = "";
	pK->v("_Mavlink", &iName);
	m_pMav = (_Mavlink*) (pK->getInst(iName));
	NULL_F(m_pMav);

	Kiss* pM = pK->child("mavMsgInterval");
	NULL_T(pM);

	int i = 0;
	while (1)
	{
		Kiss* pMI = pM->child(i++);
		if(pMI->empty())break;

		int id;
		float tInt;
		pMI->v("id", &id);
		pMI->v("tInt", &tInt);
		tInt *= USEC_1SEC;

		if(!m_pMav->setMsgInterval(id,tInt))
			LOG_E("Inteval msg id = " + i2str(id) + " not found");
	}

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
	NULL__(m_pMav,-1);

	return this->_MissionBase::check();
}

void _AP_base::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		this->_MissionBase::update();
		updateBase();

		this->autoFPSto();
	}
}

void _AP_base::updateBase(void)
{
	IF_(check()<0);

	//update Ardupilot
	m_apMode = m_pMav->m_heartbeat.m_msg.custom_mode;
	m_bApArmed = m_pMav->m_heartbeat.m_msg.base_mode & 0b10000000;

	//Attitude
	if(m_pMav->m_attitude.bReceiving(m_tStamp))
	{
		m_vAtti.x = m_pMav->m_attitude.m_msg.yaw;
		m_vAtti.y = m_pMav->m_attitude.m_msg.pitch;
		m_vAtti.z = m_pMav->m_attitude.m_msg.roll;
	}

	//get home position
	if(!m_pMav->m_homePosition.bReceiving(m_tStamp))
	{
		m_pMav->clGetHomePosition();
	}
	else
	{
		m_vHomePos.x = ((double)(m_pMav->m_homePosition.m_msg.latitude)) * 1e-7;
		m_vHomePos.y = ((double)(m_pMav->m_homePosition.m_msg.longitude)) * 1e-7;
		m_vHomePos.z = ((double)(m_pMav->m_homePosition.m_msg.altitude)) * 1e-3;
		m_bHomeSet = true;
	}

	//get position
	if(m_pMav->m_globalPositionINT.bReceiving(m_tStamp))
	{
		m_vGlobalPos.x = ((double)(m_pMav->m_globalPositionINT.m_msg.lat)) * 1e-7;
		m_vGlobalPos.y = ((double)(m_pMav->m_globalPositionINT.m_msg.lon)) * 1e-7;
		m_vGlobalPos.z = ((double)(m_pMav->m_globalPositionINT.m_msg.alt)) * 1e-3;
		m_vGlobalPos.w = ((double)(m_pMav->m_globalPositionINT.m_msg.relative_alt)) * 1e-3;
		m_apHdg = ((float)(m_pMav->m_globalPositionINT.m_msg.hdg)) * 1e-2;
	}

	if(m_pMav->m_localPositionNED.bReceiving(m_tStamp))
	{
		m_vLocalPos.x = m_pMav->m_localPositionNED.m_msg.x;
		m_vLocalPos.y = m_pMav->m_localPositionNED.m_msg.y;
		m_vLocalPos.z = m_pMav->m_localPositionNED.m_msg.z;
		m_vSpeed.x = m_pMav->m_localPositionNED.m_msg.vx;
		m_vSpeed.y = m_pMav->m_localPositionNED.m_msg.vy;
		m_vSpeed.z = m_pMav->m_localPositionNED.m_msg.vz;
	}

	//Send Heartbeat
	if(m_freqSendHeartbeat > 0 && m_tStamp - m_lastHeartbeat >= m_freqSendHeartbeat)
	{
		m_pMav->heartbeat();
		m_lastHeartbeat = m_tStamp;
	}

	m_pMav->sendSetMsgInterval();
}

void _AP_base::setApMode(uint32_t iMode)
{
	IF_(check()<0);

	mavlink_set_mode_t D;
	D.custom_mode = iMode;
	m_pMav->setMode(D);
}

uint32_t _AP_base::getApMode(void)
{
	return m_apMode;
}

string _AP_base::getApModeName(void)
{
	if(m_apMode >= AP_N_CUSTOM_MODE)return "?";

	if(m_apType == ardupilot_copter)
		return AP_COPTER_CUSTOM_MODE_NAME[m_apMode];
	else if(m_apType == ardupilot_rover)
		return AP_ROVER_CUSTOM_MODE_NAME[m_apMode];

	return "?";
}

void _AP_base::setApArm(bool bArm)
{
	IF_(check()<0);

	m_pMav->clComponentArmDisarm(bArm);
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

	m_pMav->mountControl(m.m_control);
	m_pMav->mountConfigure(m.m_config);

	mavlink_param_set_t D;
	D.param_type = MAV_PARAM_TYPE_INT8;
	string id;

	D.param_value = m.m_config.stab_pitch;
	id = "MNT_STAB_TILT";
	strcpy(D.param_id, id.c_str());
	m_pMav->param_set(D);

	D.param_value = m.m_config.stab_roll;
	id = "MNT_STAB_ROLL";
	strcpy(D.param_id,id.c_str());
	m_pMav->param_set(D);
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
	this->_MissionBase::draw();

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
			", hdg=" + f2str(m_apHdg),1);

	addMsg("Global Pos-----------------------",1);
	addMsg("lat=" + lf2str(m_vGlobalPos.x, 7) + ", lon=" + lf2str(m_vGlobalPos.y, 7),1);
	addMsg("alt=" + lf2str(m_vGlobalPos.z, 2) + ", relAlt=" + lf2str(m_vGlobalPos.w, 2),1);

	addMsg("Local Pos-------------------------",1);
	addMsg("x=" + f2str(m_pMav->m_localPositionNED.m_msg.x)+
				 ", y=" + f2str(m_pMav->m_localPositionNED.m_msg.y)+
				 ", z=" + f2str(m_pMav->m_localPositionNED.m_msg.z),1);

	addMsg("Home Pos--------------------------",1);
	addMsg("lat=" + f2str(m_vHomePos.x, 7)
			+ ", lon=" + f2str(m_vHomePos.y, 7)
			+ ", alt=" + f2str(m_vHomePos.z, 7),1);

	addMsg("Speed-----------------------------",1);
	addMsg("vx=" + f2str(m_pMav->m_localPositionNED.m_msg.vx)+
				 ", vy=" + f2str(m_pMav->m_localPositionNED.m_msg.vy)+
				 ", vz=" + f2str(m_pMav->m_localPositionNED.m_msg.vz),1);

	addMsg("System-----------------------------",1);
	addMsg("status="+i2str(m_pMav->m_heartbeat.m_msg.system_status));

	if(m_bDebug)
	{
		addMsg("Sensor----------------------------",1);
		addMsg("xAcc=" + i2str((int32_t)m_pMav->m_rawIMU.m_msg.xacc)
					 + ", yAcc=" + i2str((int32_t)m_pMav->m_rawIMU.m_msg.yacc)
					 + ", zAcc=" + i2str((int32_t)m_pMav->m_rawIMU.m_msg.zacc),1);

		addMsg("xGyro=" + i2str((int32_t)m_pMav->m_rawIMU.m_msg.xgyro)
					 + ", yGyro=" + i2str((int32_t)m_pMav->m_rawIMU.m_msg.ygyro)
					 + ", zGyro=" + i2str((int32_t)m_pMav->m_rawIMU.m_msg.zgyro),1);

		addMsg("xMag=" + i2str((int32_t)m_pMav->m_rawIMU.m_msg.xmag)
					 + ", yMag=" + i2str((int32_t)m_pMav->m_rawIMU.m_msg.ymag)
					 + ", zMag=" + i2str((int32_t)m_pMav->m_rawIMU.m_msg.zmag),1);
	}
}

}
