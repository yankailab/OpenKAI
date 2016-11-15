#include "HM_base.h"

namespace kai
{

HM_base::HM_base()
{
	m_pCAN = NULL;

	m_motorPwmL = 0;
	m_motorPwmR = 0;
	m_motorPwmW = 0;
	m_bSpeaker = 0;

	m_maxSpeedT = 65535;
	m_maxSpeedW = 2500;
	m_ctrlB0 = 0;
	m_ctrlB1 = 0;
}

HM_base::~HM_base()
{
}

bool HM_base::init(Config* pConfig)
{
	CHECK_F(this->ActionBase::init(pConfig)==false);
	pConfig->m_pInst = this;

	F_INFO(pConfig->v("maxSpeedT", &m_maxSpeedT));
	F_INFO(pConfig->v("maxSpeedW", &m_maxSpeedW));
	F_INFO(pConfig->v("bSpeaker", &m_bSpeaker));
	F_INFO(pConfig->v("motorPwmL", &m_motorPwmL));
	F_INFO(pConfig->v("motorPwmR", &m_motorPwmR));
	F_INFO(pConfig->v("motorPwmW", &m_motorPwmW));

	return true;
}

bool HM_base::link(void)
{
	NULL_F(m_pConfig);

	string iName = "";
	F_INFO(m_pConfig->v("_Canbus", &iName));
	m_pCAN = (_Canbus*) (m_pConfig->root()->getChildInstByName(&iName));

	return true;
}

void HM_base::updateCAN(void)
{
	NULL_(m_pCAN);

	unsigned long addr = 0x113;
	unsigned char cmd[8];
	const static int dint = sizeof(int)*8-1;

	m_ctrlB0 = 0;
	m_ctrlB0 |= ((m_motorPwmR >> dint) << 4);
	m_ctrlB0 |= ((m_motorPwmL >> dint) << 5);
	m_ctrlB0 |= ((m_motorPwmW >> dint) << 6);

	uint16_t motorPwmL = abs(confineVal(m_motorPwmL, m_maxSpeedT, -m_maxSpeedT));
	uint16_t motorPwmR = abs(confineVal(m_motorPwmR, m_maxSpeedT, -m_maxSpeedT));
	uint16_t motorPwmW = abs(confineVal(m_motorPwmW, m_maxSpeedW, -m_maxSpeedW));

	m_ctrlB1 = 0;
	m_ctrlB1 |= 1;						//tracktion motor relay
	m_ctrlB1 |= (1 << 1);				//working motor relay
	m_ctrlB1 |= (m_bSpeaker)?(1 << 5):0;//speaker
	m_ctrlB1 |= (1 << 6);				//external command
	m_ctrlB1 |= (1 << 7);				//0:pwm, 1:rpm

	uint16_t bFilter = 0x00ff;

	cmd[0] = m_ctrlB0;
	cmd[1] = m_ctrlB1;
	cmd[2] = motorPwmL & bFilter;
	cmd[3] = (motorPwmL>>8) & bFilter;
	cmd[4] = motorPwmR & bFilter;
	cmd[5] = (motorPwmR>>8) & bFilter;
	cmd[6] = motorPwmW & bFilter;
	cmd[7] = (motorPwmW>>8) & bFilter;

	// send data:  id = 0x00, standrad frame, data len = 8, stmp: data buf
	// CAN.sendMsgBuf(0x113, 0, 8, cmd);

	m_pCAN->send(addr, 8, cmd);
}



}
