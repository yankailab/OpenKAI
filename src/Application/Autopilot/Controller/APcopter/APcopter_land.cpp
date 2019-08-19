#include "APcopter_land.h"

namespace kai
{

APcopter_land::APcopter_land()
{
	m_altLanded = 3.0;
	m_apAlt = 0.0;
}

APcopter_land::~APcopter_land()
{
}

bool APcopter_land::init(void* pKiss)
{
	IF_F(!this->APcopter_target::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("altLanded",&m_altLanded);

	return true;
}

int APcopter_land::check(void)
{
	return this->APcopter_target::check();
}

void APcopter_land::update(void)
{
	this->ActionBase::update();
	IF_(check()<0);
	if(!bActive())
	{
		m_pDet->goSleep();
		m_vTargetPos = m_vMyPos;
		m_pPC->setPos(m_vMyPos, m_vMyPos);
		m_pPC->setON(false);
		return;
	}

	if(m_bMissionChanged)
	{
		m_pDet->wakeUp();
	}

	Land* pLD = (Land*)m_pMC->getCurrentMission();
	NULL_(pLD);

	m_apAlt = (double)(m_pAP->m_pMavlink->m_msg.global_position_int.relative_alt) * 1e-3;
	if(m_apAlt < m_altLanded)
	{
		if(m_pAP->getApMode()!=LAND)
		{
			m_pAP->setApMode(LAND);
			return;
		}

		pLD->setLanded(true);
		return;
	}

	m_pAP->setMount(m_apMount);

	m_vTargetPos.x = m_vMyPos.x; //roll
	m_vTargetPos.y = m_vMyPos.y; //pitch
	m_vTargetPos.z = pLD->m_speed;
	m_vTargetPos.w = pLD->m_hdg;

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
		m_vTargetPos.x = m_tO.m_c.x;
		m_vTargetPos.y = m_tO.m_c.y;
		m_vTargetPos.z *= (1.0 - constrain(big(abs(m_vTargetPos.x-0.5),abs(m_vTargetPos.y-0.5))*2, 0.0, 1.0));
	}

	m_pPC->setON(true);
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
		pWin->addMsg("Inactive");
		return true;
	}

	if (m_tO.m_topClass >= 0)
	{
		circle(*pMat, Point(m_tO.m_c.x * pMat->cols,
							m_tO.m_c.y * pMat->rows),
				pMat->cols * pMat->rows * 0.0001, Scalar(0, 0, 255), 2);

		pWin->addMsg("Target tag = " + i2str(m_tO.m_topClass));
	}
	else
	{
		pWin->addMsg("Target tag not found");
	}

	pWin->addMsg("apAlt/altLanded = " + f2str(m_apAlt) + "/" + f2str(m_altLanded));

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
		return true;
	}

	if (m_tO.m_topClass >= 0)
	{
		C_MSG("Target tag = " + i2str(m_tO.m_topClass));
	}
	else
	{
		C_MSG("Target tag not found");
	}

	C_MSG("apAlt/altLanded = " + f2str(m_apAlt) + "/" + f2str(m_altLanded));

	return true;
}

}
