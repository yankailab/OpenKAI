#include "APcopter_land.h"

namespace kai
{

APcopter_land::APcopter_land()
{
	m_pAP = NULL;
	m_pPC = NULL;
	m_pDet = NULL;
	m_tO.m_topClass = -1;

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

	F_INFO(pK->v("_ObjectBase", &iName));
	m_pDet = (_ObjectBase*) (pK->root()->getChildInst(iName));

	return true;
}

int APcopter_land::check(void)
{
	NULL__(m_pAP,-1);
	NULL__(m_pAP->m_pMavlink,-1);

	if(m_pDet)
	{
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
		if(m_pDet)
			m_pDet->goSleep();

		return;
	}

	if(m_bMissionChanged)
	{
		if(m_pDet)
			m_pDet->wakeUp();
	}

	Land* pLD = (Land*)m_pMC->getCurrentMission();
	NULL_(pLD);

	double speed = pLD->m_speed;

	if(m_pDet)
	{
		updateGimbal();

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

	string msg = *this->getName();
	pWin->addMsg(msg);

	if(!bActive())
	{
		msg = "Inactive";
	}
	else if (m_tO.m_topClass >= 0.0)
	{
		circle(*pMat, Point(m_tO.m_bb.x, m_tO.m_bb.y),
				pMat->cols * pMat->rows * 0.0001, Scalar(0, 0, 255), 2);

		msg = "Target tag = " + i2str(m_tO.m_topClass);
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

	if(!bActive())
	{
		C_MSG("Inactive");
	}
	else if (m_tO.m_topClass >= 0.0)
	{
		C_MSG("Target tag = " + i2str(m_tO.m_topClass));
	}
	else
	{
		C_MSG("Target tag not found");
	}

	return true;
}

}
