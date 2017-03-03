#include "HM_rth.h"

namespace kai
{

HM_rth::HM_rth()
{
	m_pHM = NULL;
	m_pKB = NULL;
	m_pPath = NULL;

	m_steerP = 0.0;
	m_rpmSteer = 0;
	m_rpmSpeed = 1500;
	m_rWP = 0.5;
	m_dHdg = 1.0;
}

HM_rth::~HM_rth()
{
}

bool HM_rth::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;
	pK->m_pInst = this;

	F_INFO(pK->v("steerP", &m_steerP));
	F_INFO(pK->v("rpmSpeed", &m_rpmSpeed));
	F_INFO(pK->v("rWP", &m_rWP));
	F_INFO(pK->v("dHdg", &m_dHdg));

	return true;
}

bool HM_rth::link(void)
{
	IF_F(!this->ActionBase::link());
	Kiss* pK = (Kiss*)m_pKiss;
	string iName;

	iName = "";
	F_ERROR_F(pK->v("HM_base", &iName));
	m_pHM = (HM_base*) (pK->parent()->getChildInstByName(&iName));

	iName = "";
	F_ERROR_F(pK->v("HM_kickBack", &iName));
	m_pKB = (HM_kickBack*) (pK->parent()->getChildInstByName(&iName));

	iName = "";
	F_ERROR_F(pK->v("_Path", &iName));
	m_pPath = (_Path*) (pK->root()->getChildInstByName(&iName));

	return true;
}

void HM_rth::update(void)
{
	this->ActionBase::update();

	NULL_(m_pHM);
	NULL_(m_pAM);
	NULL_(m_pKB);
	NULL_(m_pPath);
	IF_(!isActive());

	uint64_t tNow = get_time_usec();

	UTM_POS* pNow = m_pPath->getCurrentPos();
	NULL_(pNow);

	//Check if arrived at approach pos
	double dApproach = pNow->dist(&m_pKB->m_wpApproach);
	if(dApproach < m_rWP)
	{
		string stateName = "HM_RTH_APPROACH";
		m_pAM->transit(&stateName);
		LOG_I("Arrived at Approach Pos");
		return;
	}

	//Difference between approach pos and current pos
	UTM_POS dPos = *pNow - m_pKB->m_wpApproach;

	//Heading towards approach pos
	if(dPos.m_hdg > m_dHdg)
	{
		m_rpmSteer = m_steerP;
		if(dPos.m_hdg < 0)
			m_rpmSteer = -m_steerP;

		m_pHM->m_motorPwmL = m_rpmSteer;
		m_pHM->m_motorPwmR = -m_rpmSteer;

		return;
	}

	//Move to approach pos
	m_pHM->m_motorPwmL = m_rpmSpeed;
	m_pHM->m_motorPwmR = m_rpmSpeed;

	//TODO: compress path trajectory
	//TODO: find the closest wp towards approach position

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
