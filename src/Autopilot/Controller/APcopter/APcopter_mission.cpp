#include "../../Controller/APcopter/APcopter_mission.h"

namespace kai
{

APcopter_mission::APcopter_mission()
{
	m_pAP = NULL;
	m_tRTL = USEC_1SEC * 60 * 5;
	m_tStart = 0;
	m_iMissionSeq = UINT16_MAX;

}

APcopter_mission::~APcopter_mission()
{
}

bool APcopter_mission::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

	KISSm(pK,iMissionSeq);

	if(pK->v("tRTL",&m_tRTL))
	{
		m_tRTL *= USEC_1SEC;
	}

	string iName;

	iName = "";
	pK->v("APcopter_base", &iName);
	m_pAP = (APcopter_base*) (pK->parent()->getChildInst(iName));
	IF_Fl(!m_pAP, iName + ": not found");

	return true;
}

int APcopter_mission::check(void)
{
	NULL__(m_pAP,-1);
	NULL__(m_pAP->m_pMavlink,-1);

	return this->ActionBase::check();
}

void APcopter_mission::update(void)
{
	this->ActionBase::update();
	IF_(check()<0);

	string* pState = m_pAM->getCurrentMissionName();
	uint32_t apMode = m_pAP->apMode();

	if(apMode == FOLLOW ||
	   apMode == POSHOLD ||
	   apMode == GUIDED)
	{
		if(*pState!="CC_FOLLOW" && *pState!="CC_SEARCH")
			m_pAM->transit("CC_SEARCH");

		if(m_pAP->bApModeChanged())
		{
			m_tStart = m_tStamp;
		}
		else if(m_tStamp - m_tStart > m_tRTL)
		{
			m_pAP->setApMode(RTL);
		}
	}
	else if(apMode == LAND || apMode == RTL)
	{
		if(*pState!="CC_RTL")
			m_pAM->transit("CC_RTL");

		m_tStart = m_tStamp;
	}
	else if(apMode == AUTO && m_pAP->m_pMavlink->m_msg.mission_current.seq >= m_iMissionSeq)
	{
		m_pAP->setApMode(GUIDED);

		m_tStart = m_tStamp;
	}
	else
	{
		if(*pState!="CC_STANDBY")
			m_pAM->transit("CC_STANDBY");

		m_tStart = m_tStamp;
	}
}

bool APcopter_mission::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*)this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();
	IF_F(check()<0);

	pWin->tabNext();

	int tRTL = (int)((double)(m_tRTL - (m_tStamp - m_tStart))*OV_USEC_1SEC);
	tRTL = constrain(tRTL, 0, INT_MAX);

	string msg;
	msg = "tRTL = " + i2str(tRTL) +
		  ", mission current = " + i2str(m_pAP->m_pMavlink->m_msg.mission_current.seq) +
		  "/" + i2str(m_iMissionSeq);

	pWin->addMsg(&msg);

	pWin->tabPrev();

	return true;
}

bool APcopter_mission::cli(int& iY)
{
	IF_F(!this->ActionBase::cli(iY));
	IF_F(check()<0);

	int tRTL = (int)((double)(m_tRTL - (m_tStamp - m_tStart))*OV_USEC_1SEC);
	tRTL = constrain(tRTL, 0, INT_MAX);

	string msg;
	msg = "tRTL = " + i2str(tRTL) +
		  ", mission current = " + i2str(m_pAP->m_pMavlink->m_msg.mission_current.seq) +
		  "/" + i2str(m_iMissionSeq);
	COL_MSG;
	iY++;
	mvaddstr(iY, CLI_X_MSG, msg.c_str());

	return true;
}

}
