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

	m_pwmLow = 1000;
	m_pwmMid = 1500;
	m_pwmHigh = 2000;

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
	KISSm(pK,pwmMid);
	KISSm(pK,pwmHigh);

	pK->v("x", &m_vTarget.x);
	pK->v("y", &m_vTarget.y);
	pK->v("z", &m_vTarget.z);

	m_oTarget.init();

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
	F_INFO(pK->v("_DepthVisionBase", &iName));
	m_pDV = (_DepthVisionBase*) (pK->root()->getChildInstByName(&iName));
	IF_Fl(!m_pDV, iName + ": not found");

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
	IF_(pMav->m_msg.heartbeat.custom_mode != m_iModeEnable);

	OBJECT* pO = newFound();
	if(pO)
	{
		if(pO->m_topProb > m_oTarget.m_topProb)
		{
			m_oTarget = *pO;
			m_pTracker->updateBB(m_oTarget.m_bb);
		}
	}

	if(!m_pTracker->bTracking())
	{
		releaseRC();
		return;
	}

	vDouble4* pBB = m_pTracker->getBB();
	double pX = pBB->midX();
	double pY = pBB->midY();

	mavlink_rc_channels_override_t rc;
	rc.chan1_raw = (uint16_t)(m_pwmMid + (int)m_pPitch->update(pY, m_vTarget.y));
	rc.chan2_raw = (uint16_t)(m_pwmMid + (int)m_pRoll->update(pX, m_vTarget.x));
	rc.chan3_raw = UINT16_MAX;
	rc.chan4_raw = UINT16_MAX;
	rc.chan5_raw = UINT16_MAX;
	rc.chan6_raw = UINT16_MAX;
	rc.chan7_raw = UINT16_MAX;
	rc.chan8_raw = UINT16_MAX;
	m_pAP->m_pMavlink->rcChannelsOverride(rc);
}

OBJECT* APcopter_DNNfollow::newFound(void)
{
	IF_N(m_pDet->m_tStamp <= m_tStampDet);

	m_tStampDet = m_pDet->m_tStamp;
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
		circle(*pMat, Point(m_oTarget.m_bb.x, m_oTarget.m_bb.y),
				pMat->cols * pMat->rows * 0.0001, Scalar(0, 0, 255), 2);

		msg = "Target class = " + i2str(m_oTarget.m_topClass);
	}
	else
	{
		msg = "Target tag not found";
	}

	pWin->addMsg(&msg);

	return true;
}

bool APcopter_DNNfollow::cli(int& iY)
{
	IF_F(!this->ActionBase::cli(iY));
	IF_F(check()<0);

	string msg;
	if (m_pTracker->bTracking())
	{
		msg = "Target class = " + i2str(m_oTarget.m_topClass)
				+ ", pos = ("
				+ f2str((double)m_oTarget.m_bb.midX()) + " , "
				+ f2str((double)m_oTarget.m_bb.midY()) + ")";
	}
	else
	{
		msg = "Target tag not found";
	}

	COL_MSG;
	iY++;
	mvaddstr(iY, CLI_X_MSG, msg.c_str());

	return true;
}

}
