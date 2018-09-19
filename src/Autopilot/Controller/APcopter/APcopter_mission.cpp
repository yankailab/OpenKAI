#include "../../Controller/APcopter/APcopter_mission.h"

namespace kai
{

APcopter_mission::APcopter_mission()
{
	m_pAP = NULL;
	m_tRTL = USEC_1SEC * 60 * 5;
	m_tStart = 0;

}

APcopter_mission::~APcopter_mission()
{
}

bool APcopter_mission::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

	if(pK->v("tRTL",&m_tRTL))
	{
		m_tRTL *= USEC_1SEC;
	}

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

	return this->ActionBase::check();
}

void APcopter_mission::update(void)
{
	this->ActionBase::update();
	IF_(check()<0);

	string* pState = m_pAM->getCurrentStateName();

	switch (m_pAP->m_flightMode)
	{
	case POSHOLD:
		if(*pState!="CC_FOLLOW" || *pState!="CC_SEARCH")
			m_pAM->transit("CC_FOLLOW");
		break;
	}

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
