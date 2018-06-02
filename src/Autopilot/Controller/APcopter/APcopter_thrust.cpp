#include "APcopter_thrust.h"

namespace kai
{

APcopter_thrust::APcopter_thrust()
{
	m_pAP = NULL;
	m_pSB = NULL;
	m_pTarget.init();
}

APcopter_thrust::~APcopter_thrust()
{
}

bool APcopter_thrust::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	return true;
}

bool APcopter_thrust::link(void)
{
	IF_F(!this->ActionBase::link());
	Kiss* pK = (Kiss*) m_pKiss;
	string iName;

	iName = "";
	pK->v("APcopter_base", &iName);
	m_pAP = (APcopter_base*) (pK->parent()->getChildInstByName(&iName));
	IF_Fl(!m_pAP, iName + ": not found");

	iName = "";
	pK->v("_SlamBase", &iName);
	m_pSB = (_SlamBase*) (pK->parent()->getChildInstByName(&iName));
	IF_Fl(!m_pSB, iName + ": not found");

	return true;
}

void APcopter_thrust::update(void)
{
	this->ActionBase::update();

	IF_(!m_pSB);
	//Mavlink rc override to rc 1-4 is always alive
	//thrust only controls thrust fans


	updateMavlink();
}

void APcopter_thrust::updateMavlink(void)
{
	NULL_(m_pAP);
	NULL_(m_pAP->m_pMavlink);
	_Mavlink* pMavlink = m_pAP->m_pMavlink;

}

bool APcopter_thrust::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();

	return true;
}

}
