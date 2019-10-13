#include "_APcopter_land.h"

namespace kai
{

_APcopter_land::_APcopter_land()
{
	m_altLanded = 3.0;
	m_apAlt = 0.0;
}

_APcopter_land::~_APcopter_land()
{
}

bool _APcopter_land::init(void* pKiss)
{
	IF_F(!this->_APcopter_follow::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("altLanded",&m_altLanded);

	return true;
}

int _APcopter_land::check(void)
{
	return this->_APcopter_follow::check();
}

void _APcopter_land::update(void)
{
	this->_AutopilotBase::update();
	IF_(check()<0);
	if(!bActive())
	{
		m_pDet->goSleep();
		m_vTargetP = m_vP;
//		m_pPC->setPos(m_vP, m_vP);
		m_pPC->setEnable(false);
		return;
	}

	if(m_bMissionChanged)
	{
		m_pDet->wakeUp();
	}

	Land* pLD = (Land*)m_pMC->getCurrentMission();
	NULL_(pLD);

	m_apAlt = (double)(m_pAP->m_pMavlink->m_mavMsg.global_position_int.relative_alt) * 1e-3;
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

	m_vTargetP.x = m_vP.x; //roll
	m_vTargetP.y = m_vP.y; //pitch
	m_vTargetP.z = pLD->m_speed;
	m_vTargetP.w = pLD->m_hdg;

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
		m_vTargetP.x = m_tO.m_c.x;
		m_vTargetP.y = m_tO.m_c.y;
		m_vTargetP.z *= (1.0 - constrain(big(abs(m_vTargetP.x-0.5),abs(m_vTargetP.y-0.5))*2, 0.0, 1.0));
	}

	m_pPC->setEnable(true);
//	m_pPC->setPos(m_vP, m_vTargetP);
}

void _APcopter_land::draw(void)
{
	this->_AutopilotBase::draw();
	IF_(check()<0);

	if(!bActive())
	{
		addMsg("Inactive");
		return;
	}

	if (m_tO.m_topClass >= 0)
		addMsg("Target tag = " + i2str(m_tO.m_topClass));
	else
		addMsg("Target tag not found");

	addMsg("apAlt/altLanded = " + f2str(m_apAlt) + "/" + f2str(m_altLanded));

	IF_(!checkWindow());
	Mat* pMat = ((Window*) this->m_pWindow)->getFrame()->m();

	if (m_tO.m_topClass >= 0)
	{
		circle(*pMat, Point(m_tO.m_c.x * pMat->cols,
							m_tO.m_c.y * pMat->rows),
				pMat->cols * pMat->rows * 0.0001, Scalar(0, 0, 255), 2);
	}
}

}
