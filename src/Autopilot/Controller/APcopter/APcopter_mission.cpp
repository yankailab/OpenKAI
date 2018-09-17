#include "../../Controller/APcopter/APcopter_mission.h"

namespace kai
{

APcopter_mission::APcopter_mission()
{
	m_pAP = NULL;

}

APcopter_mission::~APcopter_mission()
{
}

bool APcopter_mission::init(void* pKiss)
{
	IF_F(this->ActionBase::init(pKiss)==false);
	Kiss* pK = (Kiss*)pKiss;

//	KISSm(pK,iClass);

	//link
	string iName;

	iName = "";
	pK->v("APcopter_base", &iName);
	m_pAP = (APcopter_base*) (pK->parent()->getChildInst(iName));
	IF_Fl(!m_pAP, iName + ": not found");

	return true;
}

int APcopter_mission::check(void)
{
	NULL__(m_pAP,-1);

	return 0;
}

void APcopter_mission::update(void)
{
	this->ActionBase::update();
	IF_(check()<0);

}

bool APcopter_mission::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*)this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();

	string msg = *this->getName();
	pWin->addMsg(&msg);

	return true;
}

}
