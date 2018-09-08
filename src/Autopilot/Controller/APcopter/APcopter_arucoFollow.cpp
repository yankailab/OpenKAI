#include "APcopter_arucoFollow.h"

namespace kai
{

APcopter_arucoFollow::APcopter_arucoFollow()
{
	m_pAP = NULL;
	m_pArUco = NULL;
	m_pDV = NULL;
	m_iModeEnable = POSHOLD;

	m_pRoll = NULL;
	m_pPitch = NULL;
	m_pYaw = NULL;
	m_pAlt = NULL;

	m_vTarget.x = 0.5;
	m_vTarget.y = 0.5;
	m_vTarget.z = 10.0;
	m_vPos.x = 0.5;
	m_vPos.y = 0.5;
	m_vPos.z = 10.0;

	m_tag = 0;
	m_angle = -1.0;
	m_bFollowing = false;

	m_pwmLow = 1000;
	m_pwmMidR = 1500;
	m_pwmMidP = 1500;
	m_pwmMidY = 1500;
	m_pwmMidA = 1500;
	m_pwmHigh = 2000;

}

APcopter_arucoFollow::~APcopter_arucoFollow()
{
}

bool APcopter_arucoFollow::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK,iModeEnable);
	KISSm(pK,pwmLow);
	KISSm(pK,pwmMidR);
	KISSm(pK,pwmMidP);
	KISSm(pK,pwmMidY);
	KISSm(pK,pwmMidA);
	KISSm(pK,pwmHigh);

	KISSm(pK,tag);
	KISSm(pK,angle);

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
	pK->v("_ArUco", &iName);
	m_pArUco = (_ObjectBase*) (pK->root()->getChildInstByName(&iName));
	IF_Fl(!m_pArUco, iName + ": not found");

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

int APcopter_arucoFollow::check(void)
{
	NULL__(m_pAP,-1);
	NULL__(m_pAP->m_pMavlink,-1);
	NULL__(m_pArUco,-1);
	NULL__(m_pRoll,-1);
	NULL__(m_pPitch,-1);
	NULL__(m_pYaw,-1);
	NULL__(m_pAlt,-1);
	NULL__(m_pArUco->m_pVision,-1);

	NULL__(m_pDV,0);

	return 1;
}

void APcopter_arucoFollow::update(void)
{
	this->ActionBase::update();

	IF_(check()<0);
	IF_(!isActive());

	_Mavlink* pMav = m_pAP->m_pMavlink;
	if(pMav->m_msg.heartbeat.custom_mode != m_iModeEnable)
	{
		releaseRC();
		if(pMav->m_msg.heartbeat.custom_mode == RTL)
		{
			string stateName = "CC_RTL";
			m_pAM->transit(&stateName);
		}

		return;
	}

	_VisionBase* pV = m_pArUco->m_pVision;
	vInt2 cSize;
	pV->info(&cSize, NULL, NULL);

	OBJECT* pO = newFound();
	if(!pO)
	{
		releaseRC();
		m_bFollowing = false;
		return;
	}

	m_bFollowing = true;
	m_vPos.x = pO->m_bb.x/(double)cSize.x;
	m_vPos.y = pO->m_bb.y/(double)cSize.y;
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

OBJECT* APcopter_arucoFollow::newFound(void)
{
	OBJECT* pO;
	int i=0;
	while((pO = m_pArUco->at(i++)) != NULL)
	{
		IF_CONT(pO->m_topClass != m_tag);

		return pO;
	}

	return NULL;
}

void APcopter_arucoFollow::releaseRC(void)
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

bool APcopter_arucoFollow::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();
	IF_F(pMat->empty());
	IF_F(check()<0);

	string msg = *this->getName() + ": ";

	if(!isActive())
	{
		msg += "Inactive";
	}
	else if (m_bFollowing)
	{
		circle(*pMat, Point(m_vPos.x * pMat->cols,
							m_vPos.y * pMat->rows),
				pMat->cols * pMat->rows * 0.00005, Scalar(0, 0, 255), 2);

		msg += "Tag pos = (" + f2str(m_vPos.x) + ", "
							   + f2str(m_vPos.y) + ", "
							   + f2str(m_vPos.z) + ")";
	}
	else
	{
		msg += "Tag not found";
	}

	pWin->addMsg(&msg);

	circle(*pMat, Point(m_vTarget.x * pMat->cols,
						m_vTarget.y * pMat->rows),
			pMat->cols * pMat->rows * 0.00005, Scalar(0, 255, 0), 2);

	return true;
}

bool APcopter_arucoFollow::cli(int& iY)
{
	IF_F(!this->ActionBase::cli(iY));
	IF_F(check()<0);

	string msg;
	if(!isActive())
	{
		msg = "Inactive";
	}
	else if (m_bFollowing)
	{
		msg = "Tag pos = (" + f2str(m_vPos.x) + ", "
							   + f2str(m_vPos.y) + ", "
							   + f2str(m_vPos.z) + ")";
	}
	else
	{
		msg = "Tag not found";
	}

	COL_MSG;
	iY++;
	mvaddstr(iY, CLI_X_MSG, msg.c_str());

	return true;
}

}
