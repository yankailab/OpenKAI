#include "HM_kickBack.h"

namespace kai
{

HM_kickBack::HM_kickBack()
{
	m_pHM = NULL;
	m_pPath = NULL;
	m_rpmBack = 0;
	m_kickBackDist = 0.0;
	m_dist = 0.0;

	reset();
}

HM_kickBack::~HM_kickBack()
{
}

void HM_kickBack::reset(void)
{
	m_wpStation.init();
	m_wpApproach.init();
	m_bSetStation = false;
}

bool HM_kickBack::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	F_INFO(pK->v("rpmBack", &m_rpmBack));
	F_INFO(pK->v("kickBackDist", &m_kickBackDist));

	return true;
}

bool HM_kickBack::link(void)
{
	IF_F(!this->ActionBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	string iName;

	iName = "";
	F_INFO(pK->v("HM_base", &iName));
	m_pHM = (HM_base*) (pK->parent()->getChildInstByName(&iName));

	iName = "";
	F_INFO(pK->v("_Path", &iName));
	m_pPath = (_Path*) (pK->root()->getChildInstByName(&iName));

	return true;
}

void HM_kickBack::update(void)
{
	this->ActionBase::update();

	NULL_(m_pHM);
	NULL_(m_pAM);
	NULL_(m_pPath);
	IF_(!isActive());

	if(!m_bSetStation)
	{
		//remember station position
		UTM_POS* pWP = m_pPath->getCurrentPos();
		NULL_(pWP);
		m_wpStation = *pWP;
		m_bSetStation = true;
		
		LOG_I("Station Pos Set");
	}

	UTM_POS* pNew = m_pPath->getCurrentPos();
	NULL_(pNew);
	m_dist = pNew->dist(&m_wpStation);

	if(m_dist >= m_kickBackDist)
	{
		//arrived at approach position
		m_wpApproach = *pNew;
		string stateName = "HM_WORK";
		m_pAM->transit(&stateName);

		LOG_I("KickBack complete, Approach Pos Set");
		return;
	}

	m_pHM->m_motorPwmL = m_rpmBack;
	m_pHM->m_motorPwmR = m_rpmBack;
	m_pHM->m_bSpeaker = true;
}

bool HM_kickBack::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->getCMat();
	IF_F(pMat->empty());

	//draw messages
	string msg;
	if (isActive())
		msg = "* ";
	else
		msg = "- ";
	msg += *this->getName() + ": dist=" + f2str(m_dist);
	pWin->addMsg(&msg);

	return true;
}

}
