#include "Traffic_alert.h"

namespace kai
{

Traffic_alert::Traffic_alert()
{
	m_pTB = NULL;
	m_tStampOB = 0;
	m_avrSpeed = 0.0;
}

Traffic_alert::~Traffic_alert()
{
}

bool Traffic_alert::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;
	pK->m_pInst = this;

	return true;
}

bool Traffic_alert::link(void)
{
	IF_F(!this->ActionBase::link());
	Kiss* pK = (Kiss*)m_pKiss;

	string iName;
	iName = "";
	pK->v("Traffic_base", &iName);
	m_pTB = (Traffic_base*) (pK->parent()->getChildInstByName(&iName));

	return true;
}

void Traffic_alert::update(void)
{
	this->ActionBase::update();

	NULL_(m_pTB);
	_ObjectBase* pOB = m_pTB->m_pOB;
	NULL_(pOB);
	IF_(pOB->m_tStamp <= m_tStampOB);
	m_tStampOB = pOB->m_tStamp;

	OBJECT* pO;
	int nCount = 0;
	double s = 0.0;
	int i=0;
	while((pO = pOB->at(i++)) != NULL)
	{
	}

	if(nCount>0)
		m_avrSpeed = s/nCount;

}

bool Traffic_alert::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*)this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();

	string msg = "Avr Speed = " + f2str(m_avrSpeed);
	pWin->addMsg(&msg);

	return true;
}

}
