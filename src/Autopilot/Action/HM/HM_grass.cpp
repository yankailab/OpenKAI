#include "HM_grass.h"

namespace kai
{

HM_grass::HM_grass()
{
	m_pHM = NULL;
	m_pIN = NULL;

	m_speedP = 0.0;
	m_steerP = 0.0;

	m_grassBoxL.init();
	m_grassBoxF.init();
	m_grassBoxR.init();

	m_grassMinProb = 0.5;
	m_grassClassIdx = 1;

	m_turnTimer = USEC_1SEC;
	m_tTurnSet = 0;
	m_rpmSteer = 0;

	m_nTurnRand = 10;
	m_tTurnRandRange = 100000;
	m_tTurnRandLen = 0;
	m_tTurnRandSet = 0;

	//	m_grassMat.init();
	//	m_grassArea.init();
}

HM_grass::~HM_grass()
{
}

bool HM_grass::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	F_INFO(pK->v("speedP", &m_speedP));
	F_INFO(pK->v("steerP", &m_steerP));
	F_INFO(pK->v("turnTimer", (int* )&m_turnTimer));
	F_INFO(pK->v("minProb", &m_grassMinProb));
	F_INFO(pK->v("iClass", &m_grassClassIdx));
	F_INFO(pK->v("nTurnRand", &m_nTurnRand));
	F_INFO(pK->v("tTurnRandRange", &m_tTurnRandRange));

	Kiss* pG;

	pG = pK->o("grassBoxL");
	IF_F(pG->empty());
	F_INFO(pG->v("left", &m_grassBoxL.m_x));
	F_INFO(pG->v("top", &m_grassBoxL.m_y));
	F_INFO(pG->v("right", &m_grassBoxL.m_z));
	F_INFO(pG->v("bottom", &m_grassBoxL.m_w));

	pG = pK->o("grassBoxF");
	IF_F(pG->empty());
	F_INFO(pG->v("left", &m_grassBoxF.m_x));
	F_INFO(pG->v("top", &m_grassBoxF.m_y));
	F_INFO(pG->v("right", &m_grassBoxF.m_z));
	F_INFO(pG->v("bottom", &m_grassBoxF.m_w));

	pG = pK->o("grassBoxR");
	IF_F(pG->empty());
	F_INFO(pG->v("left", &m_grassBoxR.m_x));
	F_INFO(pG->v("top", &m_grassBoxR.m_y));
	F_INFO(pG->v("right", &m_grassBoxR.m_z));
	F_INFO(pG->v("bottom", &m_grassBoxR.m_w));

	return true;

	//	Kiss* pG = pK->o("grassMatrix");
	//	IF_T(pG->empty());
	//	F_INFO(pG->v("nW", &m_grassMat.m_x));
	//	F_INFO(pG->v("nH", &m_grassMat.m_y));
	//	F_INFO(pG->v("left", &m_grassArea.m_x));
	//	F_INFO(pG->v("top", &m_grassArea.m_y));
	//	F_INFO(pG->v("right", &m_grassArea.m_z));
	//	F_INFO(pG->v("bottom", &m_grassArea.m_w));
}

bool HM_grass::link(void)
{
	IF_F(!this->ActionBase::link());
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

	OBJECT gBlk;

	gBlk.init();
	gBlk.m_name = "";
	gBlk.m_fBBox = m_grassBoxL;
	IF_F(!m_pIN->add(&gBlk));

	gBlk.init();
	gBlk.m_name = "";
	gBlk.m_fBBox = m_grassBoxF;
	IF_F(!m_pIN->add(&gBlk));

	gBlk.init();
	gBlk.m_name = "";
	gBlk.m_fBBox = m_grassBoxR;
	IF_F(!m_pIN->add(&gBlk));

	return true;

//	//create grass detection area instances
//	double blkW = (m_grassArea.m_z - m_grassArea.m_x) / m_grassMat.m_x;
//	double blkH = (m_grassArea.m_w - m_grassArea.m_y) / m_grassMat.m_y;
//	if (blkW <= 0 || blkH <= 0)
//	{
//		LOG_E("blkW <=0 || blkH <= 0");
//		return false;
//	}
//
//	OBJECT gBlk;
//	for (int i = 0; i < m_grassMat.m_y; i++)
//	{
//		for (int j = 0; j < m_grassMat.m_x; j++)
//		{
//			gBlk.init();
//			gBlk.m_name = "";
//			gBlk.m_fBBox.m_x = m_grassArea.m_x + j * blkW;
//			gBlk.m_fBBox.m_z = m_grassArea.m_x + (j + 1) * blkW;
//			gBlk.m_fBBox.m_y = m_grassArea.m_y + i * blkH;
//			gBlk.m_fBBox.m_w = m_grassArea.m_y + (i + 1) * blkH;
//
//			IF_F(!m_pIN->add(&gBlk));
//		}
//	}

}

void HM_grass::update(void)
{
	this->ActionBase::update();

	NULL_(m_pHM);
	NULL_(m_pAM);
	NULL_(m_pIN);

	double grassD[3];
	grassD[0] = 0.0;
	grassD[1] = 0.0;
	grassD[2] = 0.0;

	OBJECT* pObj;

	pObj = m_pIN->get(DIR_L, 0);
	if(pObj)
	{
		if (pObj->m_iClass == m_grassClassIdx)
			grassD[DIR_L] += pObj->m_prob;
	}

	pObj = m_pIN->get(DIR_F, 0);
	if(pObj)
	{
		if (pObj->m_iClass == m_grassClassIdx)
			grassD[DIR_F] += pObj->m_prob;
	}

	pObj = m_pIN->get(DIR_R, 0);
	if(pObj)
	{
		if (pObj->m_iClass == m_grassClassIdx)
			grassD[DIR_R] += pObj->m_prob;
	}

	IF_(!isActive());
	uint64_t t = get_time_usec();

	//we have/found a good place to go
	if (grassD[DIR_F] >= m_grassMinProb)
	{
		IF_(m_tTurnSet == 0);

		if(m_tTurnRandSet == 0)
		{
			m_tTurnRandLen = (rand() % m_nTurnRand) * m_tTurnRandRange;
			m_tTurnRandSet = t;
		}
		else if(t - m_tTurnRandSet > m_tTurnRandLen)
		{
			//reset the timer once finished the random extra turning
			m_tTurnSet = 0;
			m_tTurnRandSet = 0;
			return;
		}
	}

	//set new turn timer and decide the direction
	if (m_tTurnSet == 0)
	{
		m_tTurnSet = t;
		if (grassD[DIR_L] > grassD[DIR_R])
			m_rpmSteer = m_steerP;
		else
			m_rpmSteer = -m_steerP;

		return;
	}

	//not yet the time to turn
	IF_(t - m_tTurnSet < m_turnTimer);

	//keep turning
	m_pHM->m_motorPwmL = -m_rpmSteer;
	m_pHM->m_motorPwmR = m_rpmSteer;

	//	int wD = m_grassMat.m_x / 3;
	//	double grassD[3];
	//	grassD[0] = 0.0;
	//	grassD[1] = 0.0;
	//	grassD[2] = 0.0;
	//
	//	for (int i = 0; i < m_grassMat.m_y; i++)
	//	{
	//		for (int j = 0; j < m_grassMat.m_x; j++)
	//		{
	//			int k = i * m_grassMat.m_x + j;
	//			OBJECT* pObj = m_pIN->get(k, 0);
	//			NULL_(pObj);
	//
	//			if (pObj->m_iClass != m_grassClassIdx)
	//				continue;
	//
	//			grassD[j / wD] += pObj->m_prob;
	//		}
	//	}
	//
	//	double n = 1.0 / (m_grassMat.m_y * wD);
	//	grassD[0] *= n;
	//	grassD[1] *= n;
	//	grassD[2] *= n;

}

bool HM_grass::draw(void)
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
	msg += *this->getName() + ": turnTime:" + i2str((int) m_tTurnSet);
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
