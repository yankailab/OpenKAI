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

	if(!m_pTracker->bTracking())
	{
		OBJECT o;
		int nO = newFound(&o);
		if(nO == 0)
		{
			//no target found
//			m_pTracker->stopTrack();
			releaseRC();
			return;
		}

		if(nO > 0)
		{
			//found new target
			m_pTracker->startTrack(o.m_bb);
		}
	}

	vDouble4* pBB = m_pTracker->getBB();
	m_vPos.x = pBB->midX();
	m_vPos.y = pBB->midY();

	mavlink_rc_channels_override_t rc;
	rc.chan1_raw = (uint16_t)(m_pwmMidP + (int)m_pPitch->update(m_vPos.y, m_vTarget.y));
	rc.chan2_raw = (uint16_t)(m_pwmMidR + (int)m_pRoll->update(m_vPos.x, m_vTarget.x));
	rc.chan3_raw = 0;
	rc.chan4_raw = 0;
	rc.chan5_raw = 0;
	rc.chan6_raw = 0;
	rc.chan7_raw = 0;
	rc.chan8_raw = 0;
	m_pAP->m_pMavlink->rcChannelsOverride(rc);
}

int APcopter_DNNfollow::newFound(OBJECT* pObj)
{
//	if(m_pDet->m_tStamp <= m_tStampDet)
//		return -1;
//
//	m_tStampDet = m_pDet->m_tStamp;
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

	if(!tO)
		return 0;

	*pObj = *tO;
	return 1;
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

		msg = "Target pos = (" + f2str(m_vPos.x) + ", " + f2str(m_vPos.y)+")";
	}
	else
	{
		msg = "Target tag not found";
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
		msg = "Target pos = (" + f2str(m_vPos.x) + ", " + f2str(m_vPos.y)+")";
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
