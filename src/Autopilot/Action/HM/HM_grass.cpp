#include "HM_grass.h"

namespace kai
{

HM_grass::HM_grass()
{
	m_pHM = NULL;
	m_pIN = NULL;

	m_speedP = 0.0;
	m_steerP = 0.0;

	m_grassMat.init();
	m_grassArea.init();
	m_grassMinProb = 0.5;
	m_grassClassIdx = 1;
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

	string grass = "grassMatrix";
	Kiss* pG = pK->getChildByName(&grass);
	NULL_T(pG);

	F_INFO(pG->v("nW", &m_grassMat.m_x));
	F_INFO(pG->v("nH", &m_grassMat.m_y));
	F_INFO(pG->v("left", &m_grassArea.m_x));
	F_INFO(pG->v("top", &m_grassArea.m_y));
	F_INFO(pG->v("right", &m_grassArea.m_z));
	F_INFO(pG->v("bottom", &m_grassArea.m_w));
	F_INFO(pG->v("minProb", &m_grassMinProb));
	F_INFO(pG->v("iClass", &m_grassClassIdx));

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

	if (!m_pIN)
	{
		LOG_E("_ImageNet not found");
		return true;
	}

	//create grass detection area instances
	double blkW = (m_grassArea.m_z - m_grassArea.m_x) / m_grassMat.m_x;
	double blkH = (m_grassArea.m_w - m_grassArea.m_y) / m_grassMat.m_y;
	if (blkW <= 0 || blkH <= 0)
	{
		LOG_E("blkW <=0 || blkH <= 0");
		return false;
	}

	OBJECT gBlk;
	for (int i = 0; i < m_grassMat.m_y; i++)
	{
		for (int j = 0; j < m_grassMat.m_x; j++)
		{
			gBlk.init();
			gBlk.m_name = "";
			gBlk.m_fBBox.m_x = m_grassArea.m_x + j * blkW;
			gBlk.m_fBBox.m_z = m_grassArea.m_x + (j + 1) * blkW;
			gBlk.m_fBBox.m_y = m_grassArea.m_y + i * blkH;
			gBlk.m_fBBox.m_w = m_grassArea.m_y + (i + 1) * blkH;

			CHECK_F(!m_pIN->add(&gBlk));
		}
	}

	return true;
}

void HM_grass::update(void)
{
	this->ActionBase::update();

	NULL_(m_pHM);
	NULL_(m_pAM);
	NULL_(m_pIN);

	int wD = m_grassMat.m_x / 3;
	double grassD[3];
	grassD[0] = 0.0;
	grassD[1] = 0.0;
	grassD[2] = 0.0;

	for (int i = 0; i < m_grassMat.m_y; i++)
	{
		for (int j = 0; j < m_grassMat.m_x; j++)
		{
			int k = i * m_grassMat.m_x + j;
			OBJECT* pObj = m_pIN->get(k, 0);
			NULL_(pObj);

			if (pObj->m_iClass != m_grassClassIdx)
				continue;

			grassD[j / wD] += pObj->m_prob;
		}
	}

	double n = 1.0 / (m_grassMat.m_y * wD);
	grassD[0] *= n;
	grassD[1] *= n;
	grassD[2] *= n;

	CHECK_(grassD[DIR_F] >= m_grassMinProb);
	CHECK_(!isActive());

	//forward speed
	int rpmSteer = 0;
	if (grassD[DIR_L] > grassD[DIR_R])
		rpmSteer = m_steerP;
	else
		rpmSteer = -m_steerP;

	m_pHM->m_motorPwmL = -rpmSteer;
	m_pHM->m_motorPwmR = rpmSteer;

}

bool HM_grass::draw(void)
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

	NULL_T(m_pIN);
	for (int i = 0; i < m_pIN->m_iObj; i++)
	{
		OBJECT* pObj = m_pIN->get(i, 0);
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
