#include "Traffic_base.h"

namespace kai
{

Traffic_base::Traffic_base()
{
	m_pOB = NULL;
}

Traffic_base::~Traffic_base()
{
}

bool Traffic_base::init(void* pKiss)
{
	IF_F(this->ActionBase::init(pKiss)==false);
	Kiss* pK = (Kiss*)pKiss;
	pK->m_pInst = this;

//	KISSm(pK,freqAtti);

	return true;
}

bool Traffic_base::link(void)
{
	IF_F(!this->ActionBase::link());
	Kiss* pK = (Kiss*)m_pKiss;

	string iName;
	iName = "";
	pK->v("_ObjectBase", &iName);
	m_pOB = (_ObjectBase*) (pK->root()->getChildInstByName(&iName));

	return true;
}

void Traffic_base::update(void)
{
	this->ActionBase::update();

}

bool Traffic_base::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*)this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();

//	string msg = *this->getName()+": Flight Mode=" + i2str(m_flightMode);
//	pWin->addMsg(&msg);

	return true;
}

}
