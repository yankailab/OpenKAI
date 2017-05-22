#include "../../Controller/AProver/AProver_follow.h"

namespace kai
{

AProver_follow::AProver_follow()
{
	m_pAP = NULL;
	m_pAM = NULL;
	m_pUniv = NULL;

	m_destX = 0.5;
	m_destY = 0.5;
	m_destArea = 0.1;

	m_speedP = 0.0;
	m_steerP = 0.0;

	m_pTarget = NULL;
//	m_pTargetX = new kai::FilterBase();
//	m_pTargetY = new kai::FilterBase();
//	m_pTargetArea = new kai::FilterBase();
	m_filterWindow = 3;
	m_targetClass = 0;

	m_pAP = NULL;
}

AProver_follow::~AProver_follow()
{
}

bool AProver_follow::init(void* pKiss)
{
	IF_F(this->ActionBase::init(pKiss) == false);
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	F_INFO(pK->v("targetX", &m_destX));
	F_INFO(pK->v("targetY", &m_destY));
	F_INFO(pK->v("targetArea", &m_destArea));
	F_INFO(pK->v("targetClass", &m_targetClass));

	F_INFO(pK->v("speedP", &m_speedP));
	F_INFO(pK->v("steerP", &m_steerP));

	F_INFO(pK->v("filterWindow", &m_filterWindow));
//	m_pTargetX->start(m_filterWindow);
//	m_pTargetY->start(m_filterWindow);
//	m_pTargetArea->start(m_filterWindow);

	return true;
}

bool AProver_follow::link(void)
{
	IF_F(!this->ActionBase::link());
	Kiss* pK = (Kiss*) m_pKiss;
	string iName = "";

	F_INFO(pK->v("APMrover_base", &iName));
	m_pAP = (AProver_base*) (pK->parent()->getChildInstByName(&iName));

	F_INFO(pK->v("_Universe", &iName));
	m_pUniv = (_Obstacle*) (pK->root()->getChildInstByName(&iName));

	return true;
}

void AProver_follow::update(void)
{
	this->ActionBase::update();

	NULL_(m_pAP);
	NULL_(m_pUniv);
	IF_(isActive()==false);

	//get visual target and decide motion
//	m_pTarget = m_pUniv->getByClass(m_targetClass);

//	if (m_pTarget == NULL)
//	{
//		//no target found, stop and standby
//		m_pAPM->m_steer = 0;
//		m_pAPM->m_thrust = 0;
//	}
//	else
//	{
//		m_pTargetX->input(m_pTarget->m_bbox.midX());
//		m_pTargetY->input(m_pTarget->m_bbox.midY());
//		m_pTargetArea->input(m_pTarget->m_bbox.area());
//
//		//forward or backward
//		int speed = (m_destArea * m_pTarget->m_camSize.area()
//				- m_pTargetArea->v()) * m_speedP;
//
//		//steering
//		int dSteer = (m_destX * m_pTarget->m_camSize.m_x - m_pTargetX->v())
//				* (-m_steerP);
//
//		m_pAPM->m_steer = dSteer;
//		m_pAPM->m_thrust = speed;
//	}
//
//	m_pAPM->sendHeartbeat();
//	m_pAPM->sendSteerThrust();

}

bool AProver_follow::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*)this->m_pWindow;
	Mat* pMat = pWin->getFrame()->getCMat();

	string msg = *this->getName() + ": thrust=" + i2str(m_pAP->m_thrust) + ", steer=" + i2str(m_pAP->m_steer);
	pWin->addMsg(&msg);

	IF_T(m_pTarget==NULL);
	circle(*pMat, Point(m_pTarget->m_bbox.midX(), m_pTarget->m_bbox.midY()), 10,
			Scalar(0, 0, 255), 2);

	//Vehicle position
	char strBuf[128];

	sprintf(strBuf, "Attitude: Roll=%.2f, Pitch=%.2f, Yaw=%.2f",
			m_pAP->m_pMavlink->m_msg.attitude.roll,
			m_pAP->m_pMavlink->m_msg.attitude.pitch,
			m_pAP->m_pMavlink->m_msg.attitude.yaw);
	msg = strBuf;
	pWin->addMsg(&msg);

	sprintf(strBuf, "Speed: Roll=%.2f, Pitch=%.2f, Yaw=%.2f",
			m_pAP->m_pMavlink->m_msg.attitude.rollspeed,
			m_pAP->m_pMavlink->m_msg.attitude.pitchspeed,
			m_pAP->m_pMavlink->m_msg.attitude.yawspeed);
	msg = strBuf;
	pWin->addMsg(&msg);

	return true;
}

}
