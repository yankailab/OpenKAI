#include "HM_base.h"

namespace kai
{

HM_base::HM_base()
{
	m_pCAN = NULL;
	m_pCMD = NULL;
	m_pGPS = NULL;
	m_strCMD = "";
	m_rpmL = 0;
	m_rpmR = 0;
	m_motorRpmW = 0;
	m_bSpeaker = false;

	m_maxRpmT = 65535;
	m_maxRpmW = 2500;
	m_ctrlB0 = 0;
	m_ctrlB1 = 0;
	m_defaultRpmT = 3000;
	m_wheelR = 1.0;
	m_lastUpdateGPS = 0;
}

HM_base::~HM_base()
{
}

bool HM_base::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;
	pK->m_pInst = this;

	F_INFO(pK->v("maxSpeedT", &m_maxRpmT));
	F_INFO(pK->v("maxSpeedW", &m_maxRpmW));
	F_INFO(pK->v("motorRpmW", &m_motorRpmW));
	F_INFO(pK->v("defaultRpmT", &m_defaultRpmT));
	F_INFO(pK->v("wheelR", &m_wheelR));

	Kiss* pI = pK->o("cmd");
	IF_T(pI->empty());
	m_pCMD = new TCP();
	F_ERROR_F(m_pCMD->init(pI));

	return true;
}

bool HM_base::link(void)
{
	IF_F(!this->ActionBase::link());
	Kiss* pK = (Kiss*)m_pKiss;

	string iName;

	iName = "";
	F_ERROR_F(pK->v("_Canbus", &iName));
	m_pCAN = (_Canbus*) (pK->root()->getChildInstByName(&iName));

	iName = "";
	F_ERROR_F(pK->v("_GPS", &iName));
	m_pGPS = (_GPS*) (pK->root()->getChildInstByName(&iName));

	return true;
}

void HM_base::update(void)
{
	this->ActionBase::update();
	NULL_(m_pAM);
	NULL_(m_pCMD);

	updateGPS();
	updateCAN();

	string* pStateName = m_pAM->getCurrentStateName();

	if(*pStateName == "HM_STANDBY" || *pStateName == "HM_STATION" || *pStateName=="HM_FOLLOWME")
	{
		m_rpmL = 0;
		m_rpmR = 0;
	}
	else
	{
		m_rpmL = m_defaultRpmT;
		m_rpmR = m_defaultRpmT;
	}

	m_motorRpmW = 0;
	m_bSpeaker = false;

	//ignore external cmd in kickback mode
	//IF_(*pStateName == "HM_KICKBACK");
	cmd();
}

void HM_base::updateGPS(void)
{
	NULL_(m_pGPS);

	double tNow = get_time_usec();
	if(m_lastUpdateGPS==0)
	{
		m_lastUpdateGPS = tNow;
		return;
	}

	const static double tBase = 1.0/(USEC_1SEC*60);

	double dTime = (double)(tNow - m_lastUpdateGPS);
	m_lastUpdateGPS = tNow;

	//TODO: calc rpm to dist
	IF_(m_rpmL != m_rpmR);

	vDouble3 dT;
	dT.init();
	dT.m_y = m_rpmL * m_wheelR * dTime * tBase;

	m_pGPS->addTranslation(dT);
}

void HM_base::cmd(void)
{
	if (!m_pCMD->isOpen())
	{
		m_pCMD->open();
		return;
	}

	char buf;
	while (m_pCMD->read((uint8_t*) &buf, 1) > 0)
	{
		if (buf == ',')break;
		IF_CONT(buf==LF);
		IF_CONT(buf==CR);

		m_strCMD += buf;
	}

	IF_(buf!=',');

	LOG_I("CMD: "<<m_strCMD);

	string stateName;
	if(m_strCMD=="start")
	{
		stateName = *m_pAM->getCurrentStateName();

		if(stateName=="HM_STATION")
		{
			stateName = "HM_KICKBACK";
			m_pAM->transit(&stateName);
		}
		else if(stateName=="HM_STANDBY")
		{
			m_pAM->transit(m_pAM->getLastStateIdx());
		}
	}
	else if(m_strCMD=="stop")
	{
		stateName = "HM_STANDBY";
		m_pAM->transit(&stateName);
	}
	else if(m_strCMD=="work")
	{
		stateName = "HM_WORK";
		m_pAM->transit(&stateName);
	}
	else if(m_strCMD=="back_to_home")
	{
		stateName = "HM_RTH";
		m_pAM->transit(&stateName);
	}
	else if(m_strCMD=="follow_me")
	{
		stateName = "HM_FOLLOWME";
		m_pAM->transit(&stateName);
	}
	else if(m_strCMD=="station")
	{
		stateName = "HM_STATION";
		m_pAM->transit(&stateName);
		m_pGPS->reset();
	}

	m_strCMD = "";
}

void HM_base::updateCAN(void)
{
	NULL_(m_pCAN);

	unsigned long addr = 0x113;
	unsigned char cmd[8];

	m_ctrlB0 = 0;
	m_ctrlB0 |= (m_rpmR<0)?(1 << 4):0;
	m_ctrlB0 |= (m_rpmL<0)?(1 << 5):0;
	m_ctrlB0 |= (m_motorRpmW<0)?(1 << 6):0;

	uint16_t motorPwmL = abs(constrain(m_rpmL, m_maxRpmT, -m_maxRpmT));
	uint16_t motorPwmR = abs(constrain(m_rpmR, m_maxRpmT, -m_maxRpmT));
	uint16_t motorPwmW = abs(constrain(m_motorRpmW, m_maxRpmW, -m_maxRpmW));

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

bool HM_base::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->getCMat();
	NULL_F(pMat);
	IF_F(pMat->empty());

	string msg = *this->getName() + ": rpmL=" + i2str(m_rpmL)
			+ ", rpmR=" + i2str(m_rpmR);
	pWin->addMsg(&msg);

	return true;
}



}
