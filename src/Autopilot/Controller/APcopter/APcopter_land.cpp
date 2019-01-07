#include "APcopter_land.h"

namespace kai
{

APcopter_land::APcopter_land()
{
	m_pAP = NULL;
	m_pDet = NULL;

	m_mode = land_simple;
	m_tO.m_topClass = -1;
	m_orientation.x = 1.0;
	m_orientation.y = 1.0;

	m_bGimbal = false;
	m_vGimbal.init();
	m_mountControl.input_a = m_vGimbal.x * 100;	//pitch
	m_mountControl.input_b = m_vGimbal.y * 100;	//roll
	m_mountControl.input_c = m_vGimbal.z * 100;	//yaw
	m_mountControl.save_position = 0;

	m_mountConfig.stab_pitch = 0;
	m_mountConfig.stab_roll = 0;
	m_mountConfig.stab_yaw = 0;
	m_mountConfig.mount_mode = 2;
}

APcopter_land::~APcopter_land()
{
}

bool APcopter_land::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	string str;
	pK->v("mode", &str);
	if(str=="landingTarget")
		m_mode = land_apLandingTarget;
	else if(str=="posTarget")
		m_mode = land_apPosTarget;
	else
		m_mode = land_simple;

	pK->v("orientationX", &m_orientation.x);
	pK->v("orientationY", &m_orientation.y);

	KISSm(pK,bGimbal);
	Kiss* pG = pK->o("gimbal");
	if(!pG->empty())
	{
		pG->v("pitch", &m_vGimbal.x);
		pG->v("roll", &m_vGimbal.y);
		pG->v("yaw", &m_vGimbal.z);

		m_mountControl.input_a = m_vGimbal.x * 100;	//pitch
		m_mountControl.input_b = m_vGimbal.y * 100;	//roll
		m_mountControl.input_c = m_vGimbal.z * 100;	//yaw
		m_mountControl.save_position = 0;

		pG->v("stabPitch", &m_mountConfig.stab_pitch);
		pG->v("stabRoll", &m_mountConfig.stab_roll);
		pG->v("stabYaw", &m_mountConfig.stab_yaw);
		pG->v("mountMode", &m_mountConfig.mount_mode);

	}

	//link
	string iName = "";
	F_INFO(pK->v("APcopter_base", &iName));
	m_pAP = (APcopter_base*) (pK->parent()->getChildInst(iName));

	if(m_mode != land_simple)
	{
		F_ERROR_F(pK->v("_ArUco", &iName));
		m_pDet = (_ObjectBase*) (pK->root()->getChildInst(iName));
		NULL_F(m_pDet);
	}

	return true;
}

int APcopter_land::check(void)
{
	NULL__(m_pAP,-1);
	NULL__(m_pAP->m_pMavlink,-1);

	if(m_mode != land_simple)
	{
		NULL__(m_pDet,-1);
		_VisionBase* pV = m_pDet->m_pVision;
		NULL__(pV,-1);
	}

	return this->ActionBase::check();
}

void APcopter_land::update(void)
{
	this->ActionBase::update();
	IF_(check()<0);
	if(!bActive())
	{
		if(m_mode != land_simple)
		{
			m_pDet->goSleep();
		}
		return;
	}

	if(m_bMissionChanged)
	{
		if(m_mode != land_simple)
		{
			m_pDet->wakeUp();
		}
	}

	updateGimbal();

	if(m_mode == land_simple)
		updateSimple();
	else if(m_mode == land_apLandingTarget)
		updateLandingTarget();
	else if(m_mode == land_apPosTarget)
		updatePosTarget();

}

void APcopter_land::updateSimple(void)
{
	m_pAP->setApMode(LAND);
}

void APcopter_land::updateLandingTarget(void)
{
	m_pAP->setApMode(LAND);

	Land* pLD = (Land*)m_pMC->getCurrentMission();
	NULL_(pLD);

	_VisionBase* pV = m_pDet->m_pVision;
	vInt2 cSize;
	vInt2 cCenter;
	vDouble2 cAngle;
	pV->info(&cSize, &cCenter, &cAngle);

	int iDet = 0;
	OBJECT* pO = NULL;
	m_tO.m_topClass = -1;
	while(1)
	{
		pO = m_pDet->at(iDet++);
		if(!pO)break;
		if(pO->m_topClass == pLD->m_tag)
		{
			m_tO = *pO;
			break;
		}
	}

	IF_(m_tO.m_topClass < 0);

	m_D.target_num = 0;
	m_D.frame = MAV_FRAME_BODY_NED;
	m_D.distance = 0;
	m_D.size_x = 0;
	m_D.size_y = 0;

	//Change position to angles
	m_D.angle_x = (float)((double)(m_tO.m_bb.x - cCenter.x) / (double)cSize.x)
							* cAngle.x * DEG_RAD * m_orientation.x;
	m_D.angle_y = (float)((double)(m_tO.m_bb.y - cCenter.y) / (double)cSize.y)
							* cAngle.y * DEG_RAD * m_orientation.y;

	//Use depth if available
	if(pV->getType()==vision_realsense)
	{
		m_D.distance = ((_DepthVisionBase*)pV)->d(&m_tO.m_bb);
		if(m_D.distance < 0.0)
			m_D.distance = 0.0;
	}

	m_pAP->m_pMavlink->landingTarget(m_D);
}

void APcopter_land::updatePosTarget(void)
{
	Land* pLD = (Land*)m_pMC->getCurrentMission();
	NULL_(pLD);

	int iDet = 0;
	OBJECT* pO = NULL;
	m_tO.m_topClass = -1;
	while(1)
	{
		pO = m_pDet->at(iDet++);
		if(!pO)break;
		if(pO->m_topClass == pLD->m_tag)
		{
			m_tO = *pO;
			break;
		}
	}

	IF_(m_tO.m_topClass < 0);

	//TODO

}

void APcopter_land::updateGimbal(void)
{
	IF_(!m_bGimbal);

	m_pAP->m_pMavlink->mountControl(m_mountControl);
	m_pAP->m_pMavlink->mountConfigure(m_mountConfig);

	mavlink_param_set_t D;
	D.param_type = MAV_PARAM_TYPE_INT8;
	string id;

	D.param_value = m_mountConfig.stab_pitch;
	id = "MNT_STAB_TILT";
	strcpy(D.param_id, id.c_str());
	m_pAP->m_pMavlink->param_set(D);

	D.param_value = m_mountConfig.stab_roll;
	id = "MNT_STAB_ROLL";
	strcpy(D.param_id,id.c_str());
	m_pAP->m_pMavlink->param_set(D);
}

bool APcopter_land::draw(void)
{
	IF_F(!this->ActionBase::draw());
	IF_F(check()<0);

	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();

	string msg = *this->getName() + ": ";

	if(m_mode==land_simple)
		msg += "Simple";
	else if(m_mode==land_apLandingTarget)
		msg += "ApLandingTarget";
	else if(m_mode==land_apPosTarget)
		msg += "ApPosTarget";
	else
		msg += "Unknown mode";

	pWin->addMsg(msg);

	if(!bActive())
	{
		msg = "Inactive";
	}
	else if (m_tO.m_topClass >= 0.0)
	{
		circle(*pMat, Point(m_tO.m_bb.x, m_tO.m_bb.y),
				pMat->cols * pMat->rows * 0.0001, Scalar(0, 0, 255), 2);

		msg = "Target tag = " + i2str(m_tO.m_topClass)
				+ ", angle = ("
				+ f2str((double)m_D.angle_x) + " , "
				+ f2str((double)m_D.angle_y)
				+ "), d=" + f2str(m_D.distance);
	}
	else
	{
		msg = "Target tag not found";
	}

	pWin->addMsg(msg);


	return true;
}

bool APcopter_land::console(int& iY)
{
	IF_F(!this->ActionBase::console(iY));
	IF_F(check()<0);

	string msg;

	if(m_mode==land_simple)
		msg = "Simple";
	else if(m_mode==land_apLandingTarget)
		msg = "ApLandingTarget";
	else if(m_mode==land_apPosTarget)
		msg = "ApPosTarget";
	else
		msg = "Unknown mode";

	C_MSG(msg);

	if(!bActive())
	{
		msg = "Inactive";
	}
	else if (m_tO.m_topClass >= 0.0)
	{
		msg = "Target tag = " + i2str(m_tO.m_topClass)
				+ ", angle = ("
				+ f2str((double)m_D.angle_x) + " , "
				+ f2str((double)m_D.angle_y)
				+ "), d=" + f2str(m_D.distance);
	}
	else
	{
		msg = "Target tag not found";
	}

	C_MSG(msg);

	return true;
}

}
