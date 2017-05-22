#include "../../Controller/HM/HM_grass.h"

namespace kai
{

HM_grass::HM_grass()
{
	m_pHM = NULL;
	m_pIN = NULL;

	m_sequence = gt_grass;

	m_grassBoxL.init();
	m_grassBoxF.init();
	m_grassBoxR.init();
	m_grassMinProb = 0.5;

	m_turnTimer = USEC_1SEC;
	m_tTurnSet = 0;
	m_rpmSteer = 0;

	m_nTurnRand = 10;
	m_tTurnRandRange = 100000;
	m_tTurnRand = 0;

	m_iGrassClass = 1;
	m_pGrassL = NULL;
	m_pGrassF = NULL;
	m_pGrassR = NULL;

}

HM_grass::~HM_grass()
{
}

bool HM_grass::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	F_INFO(pK->v("rpmSteer", &m_rpmSteer));
	F_INFO(pK->v("turnTimer", (int* )&m_turnTimer));
	F_INFO(pK->v("minProb", &m_grassMinProb));
	F_INFO(pK->v("nTurnRand", &m_nTurnRand));
	F_INFO(pK->v("tTurnRandRange", &m_tTurnRandRange));
	F_INFO(pK->v("iGrassClass", &m_iGrassClass));

	Kiss* pG;

	pG = pK->o("grassBoxL");
	IF_F(pG->empty());
	F_INFO(pG->v("left", &m_grassBoxL.x));
	F_INFO(pG->v("top", &m_grassBoxL.y));
	F_INFO(pG->v("right", &m_grassBoxL.z));
	F_INFO(pG->v("bottom", &m_grassBoxL.w));

	pG = pK->o("grassBoxF");
	IF_F(pG->empty());
	F_INFO(pG->v("left", &m_grassBoxF.x));
	F_INFO(pG->v("top", &m_grassBoxF.y));
	F_INFO(pG->v("right", &m_grassBoxF.z));
	F_INFO(pG->v("bottom", &m_grassBoxF.w));

	pG = pK->o("grassBoxR");
	IF_F(pG->empty());
	F_INFO(pG->v("left", &m_grassBoxR.x));
	F_INFO(pG->v("top", &m_grassBoxR.y));
	F_INFO(pG->v("right", &m_grassBoxR.z));
	F_INFO(pG->v("bottom", &m_grassBoxR.w));

	return true;
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
	m_pGrassL = m_pIN->add(&gBlk);
	NULL_F(m_pGrassL);

	gBlk.init();
	gBlk.m_name = "";
	gBlk.m_fBBox = m_grassBoxF;
	m_pGrassF = m_pIN->add(&gBlk);
	NULL_F(m_pGrassF);

	gBlk.init();
	gBlk.m_name = "";
	gBlk.m_fBBox = m_grassBoxR;
	m_pGrassR = m_pIN->add(&gBlk);
	NULL_F(m_pGrassR);

	return true;
}

void HM_grass::update(void)
{
	this->ActionBase::update();

	NULL_(m_pHM);
	NULL_(m_pAM);
	NULL_(m_pIN);
	if(!isActive())
	{
		m_sequence = gt_grass;
		bSetActive(false);
		return;
	}

	bSetActive(true);
	uint64_t tNow = get_time_usec();

	if(m_sequence == gt_grass)
	{
		//do nothing if already in turning
		IF_(m_pHM->m_dir != dir_forward);

		//standby until ImageNet is ready
		if(m_pGrassF->m_iClass < 0)
		{
			m_pHM->m_rpmL = 0;
			m_pHM->m_rpmR = 0;
			return;
		}

		//on grass, keep going
		if(m_pGrassF->m_iClass == m_iGrassClass && m_pGrassF->m_prob > m_grassMinProb)
		{
			LOG_I("Grass Prob: "<<m_pGrassF->m_prob);
			return;
		}

		//out of grass ahead, set timer for delayed turning
		double probL = m_pGrassL->m_prob;
		double probR = m_pGrassR->m_prob;
		if(m_pGrassL->m_iClass != m_iGrassClass)probL = 0.0;
		if(m_pGrassR->m_iClass != m_iGrassClass)probR = 0.0;

		//if both sides are unknown, turn to the last direction
		if(m_pGrassL->m_iClass == m_iGrassClass || m_pGrassR->m_iClass == m_iGrassClass)
		{
			m_rpmSteer = abs(m_rpmSteer);
			if (probL > probR)
				m_rpmSteer *= -1;
		}

		m_tTurnSet = tNow;
		m_sequence = gt_timerSet;
		LOG_I("Sequence: timerSet");
	}

	if(m_sequence == gt_timerSet)
	{
		//not yet the time to turn
		IF_(tNow - m_tTurnSet < m_turnTimer);

		if(m_pHM->m_dir != dir_forward)
		{
			m_rpmSteer = abs(m_rpmSteer);
			if (m_pHM->m_dir == dir_left)
				m_rpmSteer *= -1;
		}

		m_sequence = gt_turn;
		LOG_I("Sequence: turn");
	}

	if(m_sequence == gt_turn)
	{
		if(m_pGrassF->m_prob <= m_grassMinProb)
		{
			//keep turning
			m_pHM->m_rpmL = m_rpmSteer;
			m_pHM->m_rpmR = -m_rpmSteer;
			return;
		}

		//start extra random turn
		m_tTurnRand = (rand() % m_nTurnRand) * m_tTurnRandRange;
		m_tTurnSet = tNow;
		m_sequence = gt_randomTurn;
		LOG_I("Sequence: randomTurn");
	}

	if(m_sequence == gt_randomTurn)
	{
		if (tNow - m_tTurnSet < m_tTurnRand)
		{
			//keep extra turning
			m_pHM->m_rpmL = m_rpmSteer;
			m_pHM->m_rpmR = -m_rpmSteer;
			return;
		}

		//reset the timer once finished the random extra turning
		m_sequence = gt_grass;
		LOG_I("Sequence: grass");
	}
}

void HM_grass::bSetActive(bool bActive)
{
	if(m_pGrassL)m_pGrassL->m_bClassify = bActive;
	if(m_pGrassF)m_pGrassF->m_bClassify = bActive;
	if(m_pGrassR)m_pGrassR->m_bClassify = bActive;
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

	Rect r;
	Scalar col;
	int bold;

	vInt42rect(&m_pGrassL->m_bbox, &r);
	col = Scalar(200, 200, 200);
	bold = 1;
	if (m_pGrassL->m_iClass == m_iGrassClass && m_pGrassL->m_prob > m_grassMinProb)
	{
		col = Scalar(0, 255, 0);
		bold = 2;
	}
	rectangle(*pMat, r, col, bold);

	vInt42rect(&m_pGrassF->m_bbox, &r);
	col = Scalar(200, 200, 200);
	bold = 1;
	if (m_pGrassF->m_iClass == m_iGrassClass && m_pGrassF->m_prob > m_grassMinProb)
	{
		col = Scalar(0, 255, 0);
		bold = 2;
	}
	rectangle(*pMat, r, col, bold);

	vInt42rect(&m_pGrassR->m_bbox, &r);
	col = Scalar(200, 200, 200);
	bold = 1;
	if (m_pGrassR->m_iClass == m_iGrassClass && m_pGrassR->m_prob > m_grassMinProb)
	{
		col = Scalar(0, 255, 0);
		bold = 2;
	}
	rectangle(*pMat, r, col, bold);

	return true;
}

}
