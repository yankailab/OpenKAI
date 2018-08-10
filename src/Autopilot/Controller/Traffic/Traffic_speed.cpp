#include "Traffic_speed.h"

namespace kai
{

Traffic_speed::Traffic_speed()
{
	m_pTB = NULL;
	m_pDF = NULL;
	m_tStampOB = 0;
	m_avrSpeed = 0.0;
	m_min = 0.0;
	m_max = 1.0;
}

Traffic_speed::~Traffic_speed()
{
}

bool Traffic_speed::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;
	pK->m_pInst = this;

	KISSm(pK, min);
	KISSm(pK, max);

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
		double a = pO->m_fBBox.area();
		IF_CONT(a < m_min);
		IF_CONT(a > m_max);

		vDouble2 vF = m_pDF->vFlow(&pO->m_fBBox);
		pO->m_speed = vF.x + vF.y;
		nCount++;
		speed += pO->m_speed;
	}

	m_avrSpeed = speed / (double)nCount;
}

bool Traffic_speed::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*)this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();

	string msg = "avrSpeed=" + f2str(m_avrSpeed);
	pWin->addMsg(&msg);

	Scalar colA = Scalar(96, 192, 192);
	Scalar colB = Scalar(128, 255, 255);
	Scalar col = colA;

	return true;
}

}
