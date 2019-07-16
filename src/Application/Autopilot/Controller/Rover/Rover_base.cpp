#include "Rover_base.h"

namespace kai
{

Rover_base::Rover_base()
{
	m_pCMD = NULL;
	m_pPIDhdg = NULL;

	m_mode = rover_idle;
	m_hdg = -1.0;
	m_targetHdg = -1.0;
	m_nSpeed = 0.0;
	m_maxSpeed = 5.0;
}

Rover_base::~Rover_base()
{
}

bool Rover_base::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

	KISSm(pK,maxSpeed);

	Kiss** ppP = pK->getChildItr();
	int i = 0;
	while (ppP[i])
	{
		Kiss* pP = ppP[i++];

		ROVER_PWM_CHANNEL p;
		p.init();
		pP->v("iChan",&p.m_iChan);
		pP->v("H",&p.m_H);
		pP->v("M",&p.m_M);
		pP->v("L",&p.m_L);
		pP->v("kDirection",&p.m_kDirection);
		pP->v("sDirection",&p.m_sDirection);

		m_vPWM.push_back(p);
	}

	string iName;
	iName = "";
	F_ERROR_F(pK->v("_RoverCMD", &iName));
	m_pCMD = (_RoverCMD*) (pK->root()->getChildInst(iName));
	NULL_Fl(m_pCMD, iName+": not found");

	iName = "";
	pK->v("PIDhdg", &iName);
	m_pPIDhdg = (PIDctrl*) (pK->root()->getChildInst(iName));
	NULL_Fl(m_pPIDhdg, iName+": not found");

	return true;
}

int Rover_base::check(void)
{
	NULL__(m_pCMD, -1);
	NULL__(m_pPIDhdg, -1);

	return 0;
}

void Rover_base::update(void)
{
	this->ActionBase::update();
	IF_(check()<0);

	m_mode = m_pCMD->m_mode;
	if(m_mode == rover_idle || m_mode == rover_manual)
	{
		m_pMC->transit("IDLE");
		setSpeed(0.0);
	}

	updatePWM();
}

void Rover_base::updatePWM(void)
{
	IF_(m_hdg < 0.0);
	IF_(m_targetHdg < 0.0);

	float dSpeed = m_pPIDhdg->update(0.0, dHdg(m_hdg, m_targetHdg), m_tStamp);
	string mission = m_pMC->getCurrentMissionName();
	if(mission == "TAG")
		dSpeed = 0.0;

	uint16_t pPWM[ROVER_N_MOTOR];
	for(int i=0; i<m_vPWM.size(); i++)
		pPWM[m_vPWM[i].m_iChan] = m_vPWM[i].updatePWM(m_nSpeed, dSpeed);

	m_pCMD->setPWM(m_vPWM.size(), pPWM);
}

void Rover_base::cmd(void)
{
}

void Rover_base::setSpeed(float nSpeed)
{
	m_nSpeed = constrain<float>(nSpeed, -1.0, 1.0);
}

void Rover_base::setHdg(float hdg)
{
	m_hdg = hdg;
}

void Rover_base::setTargetHdg(float hdg)
{
	m_targetHdg = hdg;
}

void Rover_base::setLED(uint8_t pin, uint8_t status)
{
	NULL_(m_pCMD);
	m_pCMD->pinOut(pin, status);
}

bool Rover_base::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();
	NULL_F(pMat);
	IF_F(pMat->empty());

	string msg = *this->getName()
			+ ": mode=" + c_roverModeName[m_mode]
			+ ", hdg=" + f2str(m_hdg)
			+ ", targetHdg=" + f2str(m_targetHdg)
			+ ", nSpeed=" + f2str(m_nSpeed);
	pWin->addMsg(msg);

	pWin->tabNext();

	for(int i=0;i<m_vPWM.size();i++)
	{
		ROVER_PWM_CHANNEL* pM = &m_vPWM[i];
		msg = "PWM" + i2str(i)
				+ ": H=" + i2str(pM->m_H)
				+ ", M=" + i2str(pM->m_M)
				+ ", L=" + i2str(pM->m_L)
				+ ", pwm=" + i2str(pM->m_pwm)
				;
		pWin->addMsg(msg);
	}

	pWin->tabPrev();

	return true;
}

bool Rover_base::console(int& iY)
{
	IF_F(!this->ActionBase::console(iY));
	IF_F(check()<0);

	string msg;
	C_MSG("mode=" + c_roverModeName[m_mode]
			+ ", hdg=" + f2str(m_hdg)
			+ ", targetHdg=" + f2str(m_targetHdg)
			+ ", nSpeed=" + f2str(m_nSpeed));

	for(int i=0;i<m_vPWM.size();i++)
	{
		ROVER_PWM_CHANNEL* pM = &m_vPWM[i];
		C_MSG("PWM" + i2str(i)
				+ ": H=" + i2str(pM->m_H)
				+ ", M=" + i2str(pM->m_M)
				+ ", L=" + i2str(pM->m_L)
				+ ", pwm=" + i2str(pM->m_pwm));
	}

	return true;
}

}
