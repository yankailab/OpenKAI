#include "HM_follow.h"

namespace kai
{

HM_follow::HM_follow()
{
	m_pHM = NULL;
	m_pDN = NULL;
	m_pObs = NULL;

	m_distMin = 0.0;
	m_distMax = 10.0;
	m_steerP = 0.0;
	m_speedP = 0.0;
	m_targetX = 0.5;
	m_minProb = 0.1;
	m_objLifetime = 100000;
	m_pTarget = NULL;
}

HM_follow::~HM_follow()
{
}

bool HM_follow::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	F_INFO(pK->v("distMin", &m_distMin));
	F_INFO(pK->v("distMax", &m_distMax));
	F_INFO(pK->v("speedP", &m_speedP));
	F_INFO(pK->v("steerP", &m_steerP));
	F_INFO(pK->v("targetX", &m_targetX));
	F_INFO(pK->v("minProb", &m_minProb));
	F_INFO(pK->v("objLifetime", &m_objLifetime));

	return true;
}

bool HM_follow::link(void)
{
	IF_F(!this->ActionBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	string iName = "";
	F_INFO(pK->v("HM_base", &iName));
	m_pHM = (HM_base*) (pK->parent()->getChildInstByName(&iName));

	iName = "";
	F_INFO(pK->v("_Obstacle", &iName));
	m_pObs = (_Obstacle*) (pK->root()->getChildInstByName(&iName));

	iName = "";
	F_INFO(pK->v("_DetectNet", &iName));
	m_pDN = (_DetectNet*) (pK->root()->getChildInstByName(&iName));

	if (!m_pDN)
	{
		LOG_E("_DetectNet not found");
		return true;
	}

	return true;
}

void HM_follow::update(void)
{
	this->ActionBase::update();

	NULL_(m_pHM);
	NULL_(m_pAM);
	NULL_(m_pDN);
	NULL_(m_pObs);
	if (!isActive())
	{
		m_pDN->sleep();
		return;
	}
	m_pDN->wakeUp();

	m_pTarget = NULL;

	int i;
	uint64_t t = get_time_usec() - m_objLifetime;
	for (i = 0; i < m_pDN->size(); i++)
	{
		OBJECT* pObj = m_pDN->get(i, t);
		IF_CONT(!pObj);
		IF_CONT(pObj->m_prob < m_minProb);

		pObj->m_dist = m_pObs->dist(&pObj->m_fBBox, NULL);

		if (!m_pTarget)
		{
			m_pTarget = pObj;
			continue;
		}

		if (m_pTarget->m_dist > pObj->m_dist)
		{
			m_pTarget = pObj;
			continue;
		}
	}

	NULL_(m_pTarget);
	m_pHM->m_bSpeaker = true;

	double camW = (double) m_pTarget->m_camSize.m_x;
	double pX = (camW * m_targetX - m_pTarget->m_fBBox.midX()) / camW;
	int rpmSteer = m_steerP * pX * 2;

	if(rpmSteer > 0)
	{
		m_pHM->m_motorPwmL = rpmSteer;
		m_pHM->m_motorPwmR = -rpmSteer;
		return;
	}

	double normD = constrain(m_pTarget->m_dist, m_distMin, m_distMax);
	double pD = (normD - m_distMin) / (m_distMax - m_distMin);
	int rpmSpeed = m_speedP * pD;

	m_pHM->m_motorPwmL = rpmSpeed;
	m_pHM->m_motorPwmR = rpmSpeed;

}

bool HM_follow::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->getCMat();
	NULL_F(pMat);
	IF_F(pMat->empty());

	string msg;
	if (isActive())
		msg = "* ";
	else
		msg = "- ";
	msg += *this->getName();

	if(m_pTarget)
		msg += ": dist=" + f2str(m_pTarget->m_dist);
	pWin->addMsg(&msg);

	NULL_T(m_pTarget);
	Rect r;
	vInt42rect(&m_pTarget->m_bbox, &r);
	rectangle(*pMat, r, Scalar(0, 0, 255), 2);

	return true;
}

}
