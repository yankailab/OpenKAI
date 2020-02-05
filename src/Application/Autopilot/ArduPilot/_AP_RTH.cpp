#include "../ArduPilot/_AP_RTH.h"

namespace kai
{

_AP_RTH::_AP_RTH()
{
	m_pAP = NULL;
	m_pDS = NULL;
	m_kZsensor = 1.0;
	m_apMount.init();
}

_AP_RTH::~_AP_RTH()
{
}

bool _AP_RTH::init(void* pKiss)
{
	IF_F(!this->_AutopilotBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("kZsensor", &m_kZsensor);

	Kiss* pG = pK->o("mount");
	if(!pG->empty())
	{
		double p=0, r=0, y=0;
		pG->v("pitch", &p);
		pG->v("roll", &r);
		pG->v("yaw", &y);

		m_apMount.m_control.input_a = p * 100;	//pitch
		m_apMount.m_control.input_b = r * 100;	//roll
		m_apMount.m_control.input_c = y * 100;	//yaw
		m_apMount.m_control.save_position = 0;

		pG->v("stabPitch", &m_apMount.m_config.stab_pitch);
		pG->v("stabRoll", &m_apMount.m_config.stab_roll);
		pG->v("stabYaw", &m_apMount.m_config.stab_yaw);
		pG->v("mountMode", &m_apMount.m_config.mount_mode);
	}

	string iName;

	iName = "";
	pK->v("APcopter_base", &iName);
	m_pAP = (_AP_base*) (pK->parent()->getChildInst(iName));
	IF_Fl(!m_pAP, iName + ": not found");

	iName = "";
	pK->v("_DistSensorBase", &iName);
	m_pDS = (_DistSensorBase*) (pK->root()->getChildInst(iName));

	return true;
}

int _AP_RTH::check(void)
{
	NULL__(m_pAP,-1);
	NULL__(m_pAP->m_pMavlink,-1);

	return this->_AutopilotBase::check();
}

void _AP_RTH::update(void)
{
	this->_AutopilotBase::update();
	IF_(check()<0);
	IF_(!bActive());
	RTH* pRTH = (RTH*)m_pMC->getCurrentMission();
	NULL_(pRTH);

	m_pAP->setMount(m_apMount);

	vDouble3 p;

	IF_(!m_pAP->getHomePos(&p));
	pRTH->setHome(p);

	p = m_pAP->getGlobalPos();
	pRTH->setPos(p);

	vDouble3 pHome = pRTH->getHome();
	double alt = p.z;
	if(m_pDS)
	{
		double dS = m_pDS->dAvr();
		if(dS > 0)
			alt += (pHome.z - dS) * m_kZsensor;
	}
	else
	{
		alt = pHome.z;
	}

	mavlink_set_position_target_global_int_t spt;
	spt.coordinate_frame = MAV_FRAME_GLOBAL_RELATIVE_ALT_INT;

	//target position
	spt.lat_int = pHome.x*1e7;
	spt.lon_int = pHome.y*1e7;
	spt.alt = (float)alt;

	//velocity, ignored at the moment
	spt.vx = 0.0;//(float)pWP->m_speedH;		//forward
	spt.vy = 0.0;//(float)pWP->m_speedH;		//right
	spt.vz = 0.0;//(float)vZ;					//down

	//heading
	spt.yaw_rate = (float)180.0 * DEG_RAD;
	spt.yaw = m_pAP->m_pMavlink->m_mavMsg.attitude.yaw;
	double hdg = pRTH->getHdg();
	if(hdg >= 0)
		spt.yaw = (float)hdg * DEG_RAD;

	spt.type_mask = 0b0000000111111000;
	m_pAP->m_pMavlink->setPositionTargetGlobalINT(spt);
}

void _AP_RTH::draw(void)
{
	this->_AutopilotBase::draw();
	if(!bActive())
		addMsg("Inactive",1);
	else
		addMsg("RTH",1);
}

}
