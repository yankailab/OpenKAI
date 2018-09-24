#include "APcopter_followRC.h"

namespace kai
{

APcopter_followRC::APcopter_followRC()
{
	m_pPC = NULL;
}

APcopter_followRC::~APcopter_followRC()
{
}

bool APcopter_followRC::init(void* pKiss)
{
	IF_F(!this->APcopter_followBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	//link
	string iName;

	iName = "";
	pK->v("APcopter_posCtrlRC", &iName);
	m_pPC = (APcopter_posCtrlRC*) (pK->parent()->getChildInst(iName));
	IF_Fl(!m_pPC, iName + ": not found");

	return true;
}

int APcopter_followRC::check(void)
{
	return this->APcopter_followBase::check();
}

void APcopter_followRC::update(void)
{
	this->APcopter_followBase::update();
	IF_(check()<0);
	if(!isActive())
	{
		m_pDet->sleep();
		if(m_bUseTracker)
			m_pTracker->stopTrack();

		return;
	}

	if(m_bStateChanged)
	{
		m_pDet->wakeUp();
	}

	updateGimbal();
	IF_(!find());

	vDouble4 vTarget;
	vTarget.init();
	vTarget.x = m_vTarget.x - 0.5;	//roll
	vTarget.y = m_vTarget.y - 0.5;	//pitch
	m_pPC->setPos(vTarget);

	vDouble4 vCam;
	vCam.init();
	vCam.x = 0.5;
	vCam.y = 0.5;
	m_pPC->setTargetPos(vCam);

	//Use RC override
	m_pPC->setCtrl(RC_CHAN_ROLL,true);
	m_pPC->setCtrl(RC_CHAN_PITCH,true);
}

bool APcopter_followRC::draw(void)
{
	IF_F(!this->APcopter_followBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();
	IF_F(pMat->empty());
	IF_F(check()<0);

	string msg;

	return true;
}

bool APcopter_followRC::cli(int& iY)
{
	IF_F(!this->APcopter_followBase::cli(iY));
	IF_F(check()<0);

	string msg;

	return true;
}

}
