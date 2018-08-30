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
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

//	KISSm(pK,freqAtti);

	//link
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
