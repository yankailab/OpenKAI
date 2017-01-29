#include "HM_follow.h"

namespace kai
{

HM_follow::HM_follow()
{
	m_pHM = NULL;
	m_pDN = NULL;

	m_speedP = 0.0;
	m_steerP = 0.0;

	m_targetPos.init();
	m_targetPos.m_x = 0.5;
	m_targetPos.m_y = 0.5;
	m_minProb = 0.1;
}

HM_follow::~HM_follow()
{
}

bool HM_follow::init(void* pKiss)
{
	CHECK_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	F_INFO(pK->v("speedP", &m_speedP));
	F_INFO(pK->v("steerP", &m_steerP));
	F_INFO(pK->v("targetX", &m_targetPos.m_x));
	F_INFO(pK->v("targetY", &m_targetPos.m_y));
	F_INFO(pK->v("minProb", &m_minProb));

	return true;
}

bool HM_follow::link(void)
{
	CHECK_F(!this->ActionBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	string iName = "";
	F_INFO(pK->v("HM_base", &iName));
	m_pHM = (HM_base*) (pK->parent()->getChildInstByName(&iName));

	iName = "";
	F_INFO(pK->v("_DetectNet", &iName));
	m_pDN = (_DetectNet*) (pK->root()->getChildInstByName(&iName));

	if (!m_pDN)
	{
		LOG_E("_DetectNet not found for grass navigation");
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

}

bool HM_follow::draw(void)
{
	CHECK_F(!this->ActionBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->getCMat();
	NULL_F(pMat);
	CHECK_F(pMat->empty());

	string msg;
	if(isActive())msg="* ";
	else msg="- ";
	msg += *this->getName();
	pWin->addMsg(&msg);

	NULL_T(m_pDN);
	for (int i = 0; i < m_pDN->m_iObj; i++)
	{
		OBJECT* pObj = m_pDN->get(i, 0);
		if (!pObj)
			continue;

		Rect r;
		vInt42rect(&pObj->m_bbox, &r);
		Scalar col = Scalar(200, 200, 200);
		int bold = 1;
		if (pObj->m_iClass == 1)
		{
			col = Scalar(0, 255, 0);
			bold = 2;
		}
		rectangle(*pMat, r, col, bold);
	}

	return true;
}

}
