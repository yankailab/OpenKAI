#include "../../Controller/HM/HM_rth.h"

namespace kai
{

HM_rth::HM_rth()
{
	m_pHM = NULL;
	m_pKB = NULL;
	m_pGPS = NULL;

	m_rpmSteer = 0;
	m_rpmT = 1500;
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

	F_INFO(pK->v("rpmSteer", &m_rpmSteer));
	F_INFO(pK->v("rpmT", &m_rpmT));
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
	F_ERROR_F(pK->v("_GPS", &iName));
	m_pGPS = (_GPS*) (pK->root()->getChildInstByName(&iName));

	return true;
}

void HM_rth::update(void)
{
	this->ActionBase::update();

	NULL_(m_pHM);
	NULL_(m_pAM);
	NULL_(m_pKB);
	NULL_(m_pGPS);
	IF_(!isActive());

	UTM_POS* pNow = m_pGPS->getUTM();
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

	//Heading towards approach pos
	UTM_POS dPos = m_pKB->m_wpApproach - *pNow;
	if(abs(dPos.m_hdg) > m_dHdg)
	{
		m_rpmSteer = abs(m_rpmSteer);
		if(dPos.m_hdg < 0)
			m_rpmSteer *= -1;

//		m_pHM->m_rpmL = m_rpmSteer;
//		m_pHM->m_rpmR = -m_rpmSteer;

		return;
	}

	//Move to approach pos
//	m_pHM->m_rpmL = m_rpmT;
//	m_pHM->m_rpmR = m_rpmT;
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
