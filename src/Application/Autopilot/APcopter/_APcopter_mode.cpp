#include "_APcopter_mode.h"

namespace kai
{

_APcopter_mode::_APcopter_mode()
{
	m_pAP = NULL;
}

_APcopter_mode::~_APcopter_mode()
{
}

bool _APcopter_mode::init(void* pKiss)
{
	IF_F(!this->_ActionBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

	string iName;
	iName = "";
	pK->v("APcopter_base", &iName);
	m_pAP = (_APcopter_base*) (pK->parent()->getChildInst(iName));
	IF_Fl(!m_pAP, iName + ": not found");

	Kiss** pItr = pK->getChildItr();
	AP_MODE_MISSION M;
	int i = 0;
	while (pItr[i])
	{
		Kiss* pM = pItr[i++];
		M.init();
		string n;

		n = "";
		pM->v("fromApMode",&n);
		M.m_fromApMode = m_pAP->getApModeByName(n);

		pM->v("fromApMissionSeq",&M.m_fromApMissionSeq);

		n = "";
		pM->v("toApMode",&n);
		M.m_toApMode = m_pAP->getApModeByName(n);

		n = "";
		pM->v("toMission",&n);
		M.m_toMissionIdx = m_pMC->getMissionIdx(n);

		m_vMM.push_back(M);
	}

	return true;
}

int _APcopter_mode::check(void)
{
	NULL__(m_pAP,-1);
	NULL__(m_pAP->m_pMavlink,-1);

	return this->_ActionBase::check();
}

void _APcopter_mode::update(void)
{
	this->_ActionBase::update();
	IF_(check()<0);

	int apMode = m_pAP->getApMode();
	int apMissionSeq = m_pAP->m_pMavlink->m_msg.mission_current.seq;

	for(int i=0; i<m_vMM.size(); i++)
	{
		AP_MODE_MISSION* pMM = &m_vMM[i];

		IF_CONT(pMM->m_fromApMode >= 0 &&
				pMM->m_fromApMode != apMode);

		IF_CONT(pMM->m_fromApMissionSeq >= 0 &&
				pMM->m_fromApMissionSeq != apMissionSeq);

		if(pMM->m_toApMode >= 0 && apMode != pMM->m_toApMode)
		{
			m_pAP->setApMode(pMM->m_toApMode);
		}

		if(pMM->m_toMissionIdx >= 0 && m_pAP->bApModeChanged())
		{
			m_pMC->transit(pMM->m_toMissionIdx);
		}

		LOG_I("Transit to Mission: "+i2str(pMM->m_toMissionIdx));
		return;
	}
}

bool _APcopter_mode::draw(void)
{
	IF_F(!this->_ActionBase::draw());
	Window* pWin = (Window*)this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();
	IF_F(check()<0);

	pWin->tabNext();

	pWin->addMsg("AP Mission Seq = " + i2str(m_pAP->m_pMavlink->m_msg.mission_current.seq));

	pWin->tabPrev();

	return true;
}

bool _APcopter_mode::console(int& iY)
{
	IF_F(!this->_ActionBase::console(iY));
	IF_F(check()<0);

	string msg;
	C_MSG("AP Mission Seq = " + i2str(m_pAP->m_pMavlink->m_msg.mission_current.seq));

	return true;
}

}
