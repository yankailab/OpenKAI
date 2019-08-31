#include "Rover_move.h"

namespace kai
{

Rover_move::Rover_move()
{
	m_pR = NULL;
	m_pCMD = NULL;
	m_pPIDhdg = NULL;
	m_pLineM = NULL;
	m_pLineL = NULL;
	m_dHdg = 0.0;
	m_nSpeed = 0.0;

	m_borderL = 0.0;
	m_vBorderLrange.x = 0.2;
	m_vBorderLrange.y = 0.8;
	m_borderLtarget = 0.5;
	m_kBorderLhdg = 3.0;
	m_vdHdgRange.x = -5.0;
	m_vdHdgRange.y = 5.0;

	m_bLineM = false;
	m_tCamShutter = 200000;
	m_tCamShutterStart = 0;

	m_iPinLEDtag = 21;
	m_iPinCamShutter = 10;
}

Rover_move::~Rover_move()
{
}

bool Rover_move::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

	pK->v("vBorderLrange", &m_vBorderLrange);
	pK->v("borderLtarget", &m_borderLtarget);
	pK->v("kBorderLhdg",&m_kBorderLhdg);
	pK->v("vdHdgRange",&m_vdHdgRange);
	pK->v<float>("nSpeed",&m_nSpeed);
	pK->v<uint8_t>("iPinLEDtag",&m_iPinLEDtag);
	pK->v<uint8_t>("iPinCamShutter",&m_iPinCamShutter);
	pK->v("tCamShutter",&m_tCamShutter);

	string iName;
	iName = "";
	F_ERROR_F(pK->v("Rover_base", &iName));
	m_pR = (Rover_base*) (pK->parent()->getChildInst(iName));
	NULL_Fl(m_pR, iName+": not found");

	iName = "";
	F_ERROR_F(pK->v("_RoverCMD", &iName));
	m_pCMD = (_RoverCMD*) (pK->root()->getChildInst(iName));
	NULL_Fl(m_pCMD, iName+": not found");

	iName = "";
	pK->v("PIDhdg", &iName);
	m_pPIDhdg = (PIDctrl*) (pK->root()->getChildInst(iName));
	NULL_Fl(m_pPIDhdg, iName+": not found");

	iName = "";
	F_ERROR_F(pK->v("_LineM", &iName));
	m_pLineM = (_DetectorBase*) (pK->root()->getChildInst(iName));
	NULL_Fl(m_pLineM, iName+": not found");

	iName = "";
	F_ERROR_F(pK->v("_LineL", &iName));
	m_pLineL = (_DetectorBase*) (pK->root()->getChildInst(iName));
	NULL_Fl(m_pLineL, iName+": not found");

	return true;
}

int Rover_move::check(void)
{
	NULL__(m_pR, -1);
	NULL__(m_pCMD, -1);
	NULL__(m_pPIDhdg, -1);
	NULL__(m_pLineM, -1);
	NULL__(m_pLineL, -1);

	return 0;
}

void Rover_move::update(void)
{
	this->ActionBase::update();
	IF_(check()<0);
	IF_(!bActive());

	//speed
	float nSpeed;
	switch (m_pCMD->m_mode)
	{
	case rover_forward:
		nSpeed = m_nSpeed;
		break;
	case rover_backward:
		nSpeed = -m_nSpeed;
		break;
	default:
		nSpeed = 0.0;
		break;
	}

	//mission
	string mission = m_pMC->getCurrentMissionName();
	if(mission == "IDLE")
	{
		if(m_pCMD->m_mode != rover_idle && m_pCMD->m_mode != rover_manual)
			m_pMC->transit("MOVE");

		m_pCMD->pinOut(m_iPinCamShutter,0);
		m_tCamShutterStart = 0;
	}
	else if(mission == "MOVE")
	{
		m_pR->setSpeed(nSpeed);

		m_pCMD->pinOut(m_iPinCamShutter,0);
		m_tCamShutterStart = 0;
	}
	else if(mission == "TAG")
	{
		m_pR->setSpeed(0.0);

		if(m_tCamShutterStart <= 0)
		{
			m_pCMD->pinOut(m_iPinCamShutter,1);
			m_tCamShutterStart = m_tStamp;
		}
		else if(m_tStamp - m_tCamShutterStart > m_tCamShutter)
		{
			m_pCMD->pinOut(m_iPinCamShutter,0);
			m_tCamShutterStart = m_tStamp;
		}
	}

	findLineM();

	findLineL();

}

void Rover_move::findLineM(void)
{
	IF_(check()<0);

	OBJECT* pO;
	int i=0;
	while((pO = m_pLineM->at(i++)) != NULL)
	{
		if(!m_bLineM)
		{
			m_pMC->transit("TAG");
		}
		m_bLineM = true;
		return;
	}

	m_bLineM = false;
}

void Rover_move::findLineL(void)
{
	IF_(check()<0);

	if(m_pLineL->size()>0)
		m_pCMD->pinOut(m_iPinLEDtag,1);
	else
		m_pCMD->pinOut(m_iPinLEDtag,0);

	OBJECT* pO;
	int i=0;
	while((pO = m_pLineL->at(i++)) != NULL)
	{
		float border = pO->m_bb.midY();
		IF_CONT(border < m_vBorderLrange.x);
		IF_CONT(border > m_vBorderLrange.y);

		m_borderL = border;
		m_dHdg = constrain((m_borderL-m_borderLtarget)*m_kBorderLhdg, m_vdHdgRange.x, m_vdHdgRange.y);

		m_pR->setTargetHdgDelta(m_dHdg);
		return;
	}

}

bool Rover_move::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();
	NULL_F(pMat);
	IF_F(pMat->empty());

	string msg = *this->getName()
			+ ": dHdg=" + f2str(m_dHdg)
			+ ", borderL=" + f2str(m_borderL)
			;
	pWin->addMsg(msg);

	return true;
}

bool Rover_move::console(int& iY)
{
	IF_F(!this->ActionBase::console(iY));
	IF_F(check()<0);

	string msg;
	C_MSG("dHdg=" + f2str(m_dHdg)
			+ ", borderL=" + f2str(m_borderL)
	);

	return true;
}

}
