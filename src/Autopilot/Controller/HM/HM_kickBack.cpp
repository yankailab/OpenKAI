#include "HM_kickBack.h"

namespace kai
{

HM_kickBack::HM_kickBack()
{
	m_pHM = NULL;
	m_pGPS = NULL;
	m_rpmBack = 0;
	m_rpmTurn = 0;
	m_kickBackDist = 0.0;
	m_targetHdg = 0.0;

	reset();
}

HM_kickBack::~HM_kickBack()
{
}

void HM_kickBack::reset(void)
{
	m_wpStation.init();
	m_wpApproach.init();
	m_sequence = kb_station;
}

bool HM_kickBack::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	F_INFO(pK->v("rpmBack", &m_rpmBack));
	F_INFO(pK->v("rpmTurn", &m_rpmTurn));
	F_INFO(pK->v("kickBackDist", &m_kickBackDist));
	F_INFO(pK->v("targetHdg", &m_targetHdg));

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
	F_INFO(pK->v("_GPS", &iName));
	m_pGPS = (_GPS*) (pK->root()->getChildInstByName(&iName));

	return true;
}

void HM_kickBack::update(void)
{
	this->ActionBase::update();

	NULL_(m_pHM);
	NULL_(m_pAM);
	NULL_(m_pGPS);
	IF_(m_iPriority < m_pHM->m_iPriority);

	if(!isActive())
	{
		m_sequence = kb_station;
		return;
	}

	static double rotHdg;
	UTM_POS* pNew = m_pGPS->getUTM();
	NULL_(pNew);

	if(m_sequence == kb_station)
	{
		m_pGPS->reset();
		pNew = m_pGPS->getUTM();
		m_wpStation = *pNew;
		m_sequence = kb_back;
		LOG_I("Station Pos Set");
	}

	if(m_sequence == kb_back)
	{
		LOG_I("dist:"<<pNew->dist(&m_wpStation));

		if(pNew->dist(&m_wpStation) < m_kickBackDist)
		{
			//keep back
			m_pHM->m_rpmL = m_rpmBack;
			m_pHM->m_rpmR = m_rpmBack;
			m_pHM->m_bSpeaker = true;
			return;
		}

		//arrived at approach position
		m_sequence = kb_turn;
		m_wpApproach = *pNew;

		rotHdg = dHdg(m_wpApproach.m_hdg, m_wpApproach.m_hdg + m_targetHdg);
		if(rotHdg>=0)m_rpmTurn = abs(m_rpmTurn);
		else m_rpmTurn = -abs(m_rpmTurn);

		LOG_I("Approach Pos Set");
	}

	if(m_sequence == kb_turn)
	{
		LOG_I("rotHdg:" << rotHdg << " m_hdg:" << m_wpApproach.m_hdg << " newHdg:" << pNew->m_hdg << " dHdg:" <<dHdg(m_wpApproach.m_hdg, pNew->m_hdg));


		if(abs(dHdg(m_wpApproach.m_hdg, pNew->m_hdg)) > abs(rotHdg))
		{
			//turn complete, change to work mode
			string stateName = "HM_WORK";
			m_pAM->transit(&stateName);
			m_sequence = kb_station;
			LOG_I("KickBack turn complete");
			return;
		}

		//keep turning
		m_pHM->m_rpmL = m_rpmTurn;
		m_pHM->m_rpmR = -m_rpmTurn;
		m_pHM->m_bSpeaker = true;
	}
}

bool HM_kickBack::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();
	IF_F(pMat->empty());

	//draw messages
	string msg;
	if (isActive())
		msg = "* ";
	else
		msg = "- ";
	msg += *this->getName();
	pWin->addMsg(&msg);

	return true;
}

}
