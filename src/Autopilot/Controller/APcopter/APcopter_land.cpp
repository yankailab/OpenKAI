#include "APcopter_land.h"

namespace kai
{

APcopter_land::APcopter_land()
{
	m_pAP = NULL;
	m_pPC = NULL;
	m_pDet = NULL;
	m_tO.m_topClass = -1;

	m_vMyPos.init();
	m_vMyPos.x = 0.5;
	m_vMyPos.y = 0.5;
	m_vTargetPos.init();

	m_mountControl.input_a = 0;	//pitch
	m_mountControl.input_b = 0;	//roll
	m_mountControl.input_c = 0;	//yaw
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

	Kiss* pG = pK->o("gimbal");
	if(!pG->empty())
	{
		double p=0, r=0, y=0;
		pG->v("pitch", &p);
		pG->v("roll", &r);
		pG->v("yaw", &y);

		m_mountControl.input_a = p * 100;	//pitch
		m_mountControl.input_b = r * 100;	//roll
		m_mountControl.input_c = y * 100;	//yaw
		m_mountControl.save_position = 0;

		pG->v("stabPitch", &m_mountConfig.stab_pitch);
		pG->v("stabRoll", &m_mountConfig.stab_roll);
		pG->v("stabYaw", &m_mountConfig.stab_yaw);
		pG->v("mountMode", &m_mountConfig.mount_mode);
	}

	pG = pK->o("targetPos");
	if(!pG->empty())
	{
		pG->v("x", &m_vTargetPos.x);
		pG->v("y", &m_vTargetPos.y);
		pG->v("z", &m_vTargetPos.z);
		pG->v("w", &m_vTargetPos.w);
	}

	pG = pK->o("myPos");
	if(!pG->empty())
	{
		pG->v("x", &m_vMyPos.x);
		pG->v("y", &m_vMyPos.y);
		pG->v("z", &m_vMyPos.z);
		pG->v("w", &m_vMyPos.w);
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

	m_vTargetPos.x = -1;	//roll
	m_vTargetPos.y = -1;	//pitch
	m_vTargetPos.z = pLD->m_speed;
	m_vTargetPos.w = pLD->m_hdg;

	if(m_pDet)
	{
		m_pAP->setGimbal(m_mountControl, m_mountConfig);

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

		if(m_tO.m_topClass >= 0)
		{
			m_vTargetPos.x = m_tO.m_bb.x;
			m_vTargetPos.y = m_tO.m_bb.y;
			m_vTargetPos.z *= (1.0 - constrain(big(abs(m_vTargetPos.x-0.5),abs(m_vTargetPos.y-0.5))*2, 0.0, 1.0));
		}
	}

	m_pPC->setPos(m_vMyPos, m_vTargetPos);

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
	else if (m_tO.m_topClass >= 0)
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
	else if (m_tO.m_topClass >= 0)
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
