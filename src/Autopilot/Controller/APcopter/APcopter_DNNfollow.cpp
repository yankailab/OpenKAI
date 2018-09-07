#include "APcopter_DNNfollow.h"

namespace kai
{

APcopter_DNNfollow::APcopter_DNNfollow()
{
	m_pAP = NULL;
	m_pDet = NULL;
	m_pDV = NULL;
	m_pTracker = NULL;
	m_iModeEnable = POSHOLD;
	m_tStampDet = 0;

	m_pRoll = NULL;
	m_pPitch = NULL;
	m_pYaw = NULL;
	m_pAlt = NULL;

	m_iClass = -1;
	m_vTarget.x = 0.5;
	m_vTarget.y = 0.5;
	m_vTarget.z = 10.0;
	m_vPos.x = 0.5;
	m_vPos.y = 0.5;
	m_vPos.z = 10.0;

	m_pwmLow = 1000;
	m_pwmMidR = 1500;
	m_pwmMidP = 1500;
	m_pwmMidY = 1500;
	m_pwmMidA = 1500;
	m_pwmHigh = 2000;

	m_timeOn = 0;
	m_timeOut = USEC_1SEC * 180;

}

APcopter_DNNfollow::~APcopter_DNNfollow()
{
}

bool APcopter_DNNfollow::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK,iModeEnable);
	KISSm(pK,iClass);
	KISSm(pK,pwmLow);
	KISSm(pK,pwmMidR);
	KISSm(pK,pwmMidP);
	KISSm(pK,pwmMidY);
	KISSm(pK,pwmMidA);
	KISSm(pK,pwmHigh);

	KISSm(pK,timeOut);
	m_timeOut *= USEC_1SEC;

	pK->v("x", &m_vTarget.x);
	pK->v("y", &m_vTarget.y);
	pK->v("z", &m_vTarget.z);

	//link
	string iName;

	iName = "";
	pK->v("APcopter_base", &iName);
	m_pAP = (APcopter_base*) (pK->parent()->getChildInstByName(&iName));
	IF_Fl(!m_pAP, iName + ": not found");

	iName = "";
	pK->v("_ObjectBase", &iName);
	m_pDet = (_ObjectBase*) (pK->root()->getChildInstByName(&iName));
	IF_Fl(!m_pDet, iName + ": not found");

	iName = "";
	pK->v("_TrackerBase", &iName);
	m_pTracker = (_TrackerBase*) (pK->root()->getChildInstByName(&iName));
	IF_Fl(!m_pTracker, iName + ": not found");

	iName = "";
	F_INFO(pK->v("_DepthVisionBase", &iName));
	m_pDV = (_DepthVisionBase*) (pK->root()->getChildInstByName(&iName));

	iName = "";
	pK->v("PIDroll", &iName);
	m_pRoll = (PIDctrl*) (pK->root()->getChildInstByName(&iName));
	IF_Fl(!m_pRoll, iName + ": not found");

	iName = "";
	pK->v("PIDpitch", &iName);
	m_pPitch = (PIDctrl*) (pK->root()->getChildInstByName(&iName));
	IF_Fl(!m_pPitch, iName + ": not found");

	iName = "";
	pK->v("PIDyaw", &iName);
	m_pYaw = (PIDctrl*) (pK->root()->getChildInstByName(&iName));
	IF_Fl(!m_pYaw, iName + ": not found");

	iName = "";
	pK->v("PIDalt", &iName);
	m_pAlt = (PIDctrl*) (pK->root()->getChildInstByName(&iName));
	IF_Fl(!m_pAlt, iName + ": not found");

	return true;
}

int APcopter_DNNfollow::check(void)
{
	NULL__(m_pAP,-1);
	NULL__(m_pAP->m_pMavlink,-1);
	NULL__(m_pDet,-1);
	NULL__(m_pRoll,-1);
	NULL__(m_pPitch,-1);
	NULL__(m_pYaw,-1);
	NULL__(m_pAlt,-1);
	NULL__(m_pTracker,-1);

	NULL__(m_pDV,0);

	return 1;
}

void APcopter_DNNfollow::update(void)
{
	this->ActionBase::update();

	IF_(check()<0);

	_Mavlink* pMav = m_pAP->m_pMavlink;
	if(pMav->m_msg.heartbeat.custom_mode != m_iModeEnable)
	{
		releaseRC();
		return;
	}

	if(m_timeOn == 0)
	{
		m_timeOn = m_tStamp;
	}
	else if(m_tStamp - m_timeOn >= m_timeOut)
	{
		releaseRC();
		mavlink_set_mode_t D;
		D.custom_mode = RTL;
		pMav->setMode(D);
		m_pTracker->stopTrack();
		return;
	}

	OBJECT* pO = newFound();
	if(!m_pTracker->bTracking())
	{
		if(pO)
			m_pTracker->startTrack(pO->m_bb);

		releaseRC();
		return;
	}

	vDouble4* pBB = m_pTracker->getBB();
	m_vPos.x = pBB->midX();
	m_vPos.y = pBB->midY();
	m_vPos.z = (double)pMav->m_msg.global_position_int.relative_alt * 0.001;

	mavlink_rc_channels_override_t rc;
	rc.chan1_raw = (uint16_t)(m_pwmMidP + (int)m_pPitch->update(m_vPos.y, m_vTarget.y, m_vPos.z));
	rc.chan2_raw = (uint16_t)(m_pwmMidR + (int)m_pRoll->update(m_vPos.x, m_vTarget.x, m_vPos.z));
	rc.chan3_raw = 0;
	rc.chan4_raw = 0;
	rc.chan5_raw = 0;
	rc.chan6_raw = 0;
	rc.chan7_raw = 0;
	rc.chan8_raw = 0;
	m_pAP->m_pMavlink->rcChannelsOverride(rc);
}

OBJECT* APcopter_DNNfollow::newFound(void)
{
	OBJECT* pO;
	OBJECT* tO = NULL;
	double topProb = 0.0;
	int i=0;
	while((pO = m_pDet->at(i++)) != NULL)
	{
		IF_CONT(pO->m_topClass != m_iClass);
		IF_CONT(pO->m_topProb < topProb);

		tO = pO;
		topProb = pO->m_topProb;
	}

	return tO;
}

void APcopter_DNNfollow::releaseRC(void)
{
	mavlink_rc_channels_override_t rc;
	rc.chan1_raw = 0;
	rc.chan2_raw = 0;
	rc.chan3_raw = 0;
	rc.chan4_raw = 0;
	rc.chan5_raw = 0;
	rc.chan6_raw = 0;
	rc.chan7_raw = 0;
	rc.chan8_raw = 0;
	m_pAP->m_pMavlink->rcChannelsOverride(rc);
}

bool APcopter_DNNfollow::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();
	IF_F(pMat->empty());
	IF_F(check()<0);

	string msg;
	if (m_pTracker->bTracking())
	{
		circle(*pMat, Point(m_vPos.x * pMat->cols,
							m_vPos.y * pMat->rows),
				pMat->cols * pMat->rows * 0.00005, Scalar(0, 0, 255), 2);

		msg = "Target pos = (" + f2str(m_vPos.x) + ", "
							   + f2str(m_vPos.y) + ", "
							   + f2str(m_vPos.z) + ")";
	}
	else
	{
		msg = "Target not found";
	}

	pWin->addMsg(&msg);

	circle(*pMat, Point(m_vTarget.x * pMat->cols,
						m_vTarget.y * pMat->rows),
			pMat->cols * pMat->rows * 0.00005, Scalar(0, 255, 0), 2);

	return true;
}

bool APcopter_DNNfollow::cli(int& iY)
{
	IF_F(!this->ActionBase::cli(iY));
	IF_F(check()<0);

	string msg;
	if (m_pTracker->bTracking())
	{
		msg = "Target pos = (" + f2str(m_vPos.x) + ", "
							   + f2str(m_vPos.y) + ", "
							   + f2str(m_vPos.z) + ")";
	}
	else
	{
		msg = "Target not found";
	}

	COL_MSG;
	iY++;
	mvaddstr(iY, CLI_X_MSG, msg.c_str());

	return true;
}

}
