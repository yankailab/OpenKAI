#include "HM_grass.h"

namespace kai
{

HM_grass::HM_grass()
{
	m_pHM = NULL;
	m_pIN = NULL;

	m_speedP = 0.0;
	m_steerP = 0.0;
}

HM_grass::~HM_grass()
{
}

bool HM_grass::init(void* pKiss)
{
	CHECK_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	F_INFO(pK->v("speedP", &m_speedP));
	F_INFO(pK->v("steerP", &m_steerP));

	return true;
}

bool HM_grass::link(void)
{
	CHECK_F(!this->ActionBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	string iName = "";
	F_INFO(pK->v("HM_base", &iName));
	m_pHM = (HM_base*) (pK->parent()->getChildInstByName(&iName));

	iName = "";
	F_INFO(pK->v("_ImageNet", &iName));
	m_pIN = (_ImageNet*) (pK->root()->getChildInstByName(&iName));

	NULL_F(m_pIN);

	//create grass detection area instances
	Kiss** pItr = pK->getChildItr();
	OBJECT areaObj;
	int	nArea = 0;

	int i = 0;
	while (pItr[i])
	{
		Kiss* pArea = pItr[i++];

		areaObj.init();
		if(!pArea->v("area", &areaObj.m_name))continue;
		if(!pArea->v("left", &areaObj.m_fBBox.m_x))continue;
		if(!pArea->v("top", &areaObj.m_fBBox.m_y))continue;
		if(!pArea->v("right", &areaObj.m_fBBox.m_z))continue;
		if(!pArea->v("bottom", &areaObj.m_fBBox.m_w))continue;

		if(m_pIN->add(&areaObj))nArea++;

		if(nArea>=3)break;
	}

	if(nArea<3)
	{
		LOG_E("3 areas needed for grass navigation");
		return false;
	}

	return true;
}

void HM_grass::update(void)
{
	this->ActionBase::update();

	NULL_(m_pHM);
	NULL_(m_pAM);
	NULL_(m_pIN);

	for (int i = 0; i < m_pIN->m_iObj; i++)
	{
		OBJECT* pObj = m_pIN->get(i,0);
		if(!pObj)continue;

	}

}

bool HM_grass::draw(void)
{
	CHECK_F(!this->ActionBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->getCMat();
	NULL_F(pMat);
	CHECK_F(pMat->empty());

	string msg = *this->getName() + ": rpmL=" + i2str(m_pHM->m_motorPwmL) + ", rpmR="
			+ i2str(m_pHM->m_motorPwmR);
	pWin->addMsg(&msg);

	NULL_T(m_pIN);
	for (int i = 0; i < m_pIN->m_iObj; i++)
	{
		OBJECT* pObj = m_pIN->get(i,0);
		if(!pObj)continue;

		Rect r;
		vInt42rect(&pObj->m_bbox,&r);
		Scalar col = Scalar(0,255,0);
		int bold = 1;
//		if(m_distM < m_alertDist)
//		{
//			col = Scalar(0,0,255);
//			bold = 2;
//		}
		rectangle(*pMat, r, col, bold);
	}


	return true;
}

}
