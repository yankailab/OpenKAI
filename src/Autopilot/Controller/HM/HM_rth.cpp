#include "HM_rth.h"

namespace kai
{

HM_rth::HM_rth()
{
	m_pHM = NULL;

	m_rpmSteer = 0;
	m_rpmT = 1500;
}

HM_rth::~HM_rth()
{
}

bool HM_rth::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;
	pK->m_pInst = this;

	F_INFO(pK->v("rpmSteer", &m_rpmSteer));
	F_INFO(pK->v("rpmT", &m_rpmT));

	//link
	string iName;

	iName = "";
	F_ERROR_F(pK->v("HM_base", &iName));
	m_pHM = (HM_base*) (pK->parent()->getChildInstByName(&iName));

	return true;
}

void HM_rth::update(void)
{
	this->ActionBase::update();

	NULL_(m_pHM);
	NULL_(m_pAM);
	IF_(m_iPriority < m_pHM->m_iPriority);
	IF_(!isActive());

}

bool HM_rth::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*)this->m_pWindow;

	//draw messages
	string msg;
	if (isActive())
		msg = "* ";
	else
		msg = "- ";
	msg += *this->getName();
	pWin->addMsg(&msg);

	return true;
}

}
