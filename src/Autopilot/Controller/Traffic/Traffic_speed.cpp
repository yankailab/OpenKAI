#include "Traffic_speed.h"

namespace kai
{

Traffic_speed::Traffic_speed()
{
	m_pTB = NULL;
	m_pDF = NULL;
	m_tStampOB = 0;
	m_avrSpeed = 0.0;
	m_minArea = 0.0;
	m_maxArea = 1.0;
	m_bDrawObjVopt = false;
	m_drawVscale = 1.0;

}

Traffic_speed::~Traffic_speed()
{
}

bool Traffic_speed::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;
	pK->m_pInst = this;

	KISSm(pK, minArea);
	KISSm(pK, maxArea);

	KISSm(pK, bDrawObjVopt);
	KISSm(pK, drawVscale);

	return true;
}

bool Traffic_speed::link(void)
{
	IF_F(!this->ActionBase::link());
	Kiss* pK = (Kiss*)m_pKiss;

	string iName;

	iName = "";
	pK->v("Traffic_base", &iName);
	m_pTB = (Traffic_base*) (pK->parent()->getChildInstByName(&iName));

	iName = "";
	pK->v("_DenseFlow", &iName);
	m_pDF = (_DenseFlow*) (pK->root()->getChildInstByName(&iName));

	return true;
}

void Traffic_speed::update(void)
{
	this->ActionBase::update();

	NULL_(m_pDF);
	NULL_(m_pTB);
	_ObjectBase* pOB = m_pTB->m_pOB;
	NULL_(pOB);
	IF_(pOB->m_tStamp <= m_tStampOB);
	m_tStampOB = pOB->m_tStamp;

	OBJECT* pO;
	int i=0;
	int nCount = 0;
	double speed = 0.0;
	while((pO = pOB->at(i++)) != NULL)
	{
		double a = pO->m_bb.area();
		IF_CONT(a < m_minArea);
		IF_CONT(a > m_maxArea);

		pO->m_vOpt = m_pDF->vFlow(&pO->m_bb);
		nCount++;
		speed += pO->m_vOpt.len();
	}

	m_avrSpeed = speed / (double)nCount;
}

bool Traffic_speed::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*)this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();
	IF_F(pMat->empty());

	NULL_F(m_pTB);
	_ObjectBase* pOB = m_pTB->m_pOB;
	NULL_F(pOB);
	NULL_F(pOB->m_pVision);

	string msg = "avrSpeed=" + f2str(m_avrSpeed);
	pWin->addMsg(&msg);

	Scalar colA = Scalar(228, 228, 128);
	Scalar colB = Scalar(255, 255, 128);
	vInt2 cSize = pOB->m_pVision->getSize();

	OBJECT* pO;
	int i=0;
	while((pO = pOB->at(i++)) != NULL)
	{
		vInt4 iBB = pO->iBBox(cSize);
		Point pC = Point(iBB.midX(), iBB.midY());

		Scalar col = colA;

		//bbox
		Rect r;
		vInt42rect(iBB, r);
		rectangle(*pMat, r, col, 1);

		//vOpt
		Point pV = Point(pO->m_vOpt.x * m_drawVscale, pO->m_vOpt.y * m_drawVscale);
		line(*pMat, pC, pC - pV, col, 1);

	}

	return true;
}

}
