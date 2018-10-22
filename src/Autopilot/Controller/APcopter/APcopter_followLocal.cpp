#include "APcopter_followLocal.h"

namespace kai
{

APcopter_followLocal::APcopter_followLocal()
{
	m_pPC = NULL;
	m_vCam.init();
	m_vCam.x = 0.5;
	m_vCam.y = 0.5;
}

APcopter_followLocal::~APcopter_followLocal()
{
}

bool APcopter_followLocal::init(void* pKiss)
{
	IF_F(!this->APcopter_followBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("camX", &m_vCam.x);
	pK->v("camY", &m_vCam.y);
	pK->v("camYaw", &m_vCam.w);

	//link
	string iName;

	iName = "";
	pK->v("APcopter_posCtrlBase", &iName);
	m_pPC = (APcopter_posCtrlBase*) (pK->parent()->getChildInst(iName));
	IF_Fl(!m_pPC, iName + ": not found");

	return true;
}

int APcopter_followLocal::check(void)
{
	return this->APcopter_followBase::check();
}

void APcopter_followLocal::update(void)
{
	this->APcopter_followBase::update();
	IF_(check()<0);
	if(!isActive())
	{
		m_pDet->goSleep();
		if(m_bUseTracker)
		{
			m_pTracker[0]->stopTrack();
			m_pTracker[1]->stopTrack();
		}

		return;
	}

	if(m_bStateChanged)
	{
		m_pDet->wakeUp();
	}

	updateGimbal();
	IF_(!find());

	m_pPC->setPos(m_vTarget);
	m_pPC->setTargetPos(m_vCam);
}

bool APcopter_followLocal::draw(void)
{
	IF_F(!this->APcopter_followBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();
	IF_F(pMat->empty());
	IF_F(check()<0);

	string msg;

	return true;
}

bool APcopter_followLocal::cli(int& iY)
{
	IF_F(!this->APcopter_followBase::cli(iY));
	IF_F(check()<0);

	string msg;

	return true;
}

}
