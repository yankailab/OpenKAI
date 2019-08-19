#include "APcopter_WP.h"

namespace kai
{

APcopter_WP::APcopter_WP()
{
	m_pAP = NULL;
	m_pDS = NULL;
	m_dZdefault = 0.0;
	m_kZsensor = 1.0;
	m_apMount.init();
}

APcopter_WP::~APcopter_WP()
{
}

bool APcopter_WP::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("dZdefault",&m_dZdefault);
	pK->v("kZsensor",&m_kZsensor);

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
	m_pAP = (APcopter_base*) (pK->parent()->getChildInst(iName));
	IF_Fl(!m_pAP, iName + ": not found");

	iName = "";
	pK->v("_DistSensorBase", &iName);
	m_pDS = (_DistSensorBase*) (pK->root()->getChildInst(iName));

	return true;
}

int APcopter_WP::check(void)
{
	NULL__(m_pAP,-1);
	NULL__(m_pAP->m_pMavlink,-1);

	return this->ActionBase::check();
}

void APcopter_WP::update(void)
{
	this->ActionBase::update();
	IF_(check()<0);
	IF_(!bActive());
	Waypoint* pWP = (Waypoint*)m_pMC->getCurrentMission();
	NULL_(pWP);

	m_pAP->setMount(m_apMount);

	double alt = (double)(m_pAP->m_pMavlink->m_msg.global_position_int.relative_alt) * 1e-3;
	if(m_pDS)
	{
		double dS = m_pDS->dAvr();
		if(dS > 0)
			alt += (pWP->m_vWP.z - dS) * m_kZsensor;
		else
			alt += m_dZdefault;
	}
	else
	{
		alt = pWP->m_vWP.z;
	}

	mavlink_set_position_target_global_int_t spt;
	spt.coordinate_frame = MAV_FRAME_GLOBAL_RELATIVE_ALT_INT;

	//target position
	spt.lat_int = pWP->m_vWP.x*1e7;
	spt.lon_int = pWP->m_vWP.y*1e7;
	spt.alt = (float)alt;

	//velocity, ignored at the moment
	spt.vx = 0.0;//(float)pWP->m_speedH;		//forward
	spt.vy = 0.0;//(float)pWP->m_speedH;		//right
	spt.vz = 0.0;//(float)vZ;					//down

	//heading
	spt.yaw_rate = (float)180.0 * DEG_RAD;
	spt.yaw = m_pAP->m_pMavlink->m_msg.attitude.yaw;
	if(pWP->m_hdg >= 0)
		spt.yaw = (float)pWP->m_hdg * DEG_RAD;

	spt.type_mask = 0b0000000111111000;

	m_pAP->m_pMavlink->setPositionTargetGlobalINT(spt);
}

bool APcopter_WP::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();
	IF_F(pMat->empty());
	IF_F(check()<0);

	pWin->tabNext();

	if(!bActive())
		pWin->addMsg("Inactive");
	else
		pWin->addMsg("Waypoint");

	pWin->tabPrev();

	return true;
}

bool APcopter_WP::console(int& iY)
{
	IF_F(!this->ActionBase::console(iY));
	IF_F(check()<0);

	string msg;

	if(!bActive())
	{
		C_MSG("Inactive");
	}
	else
	{
		C_MSG("Waypoint");
	}

	return true;
}

}
