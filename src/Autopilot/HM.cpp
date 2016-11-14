#include "HM.h"

namespace kai
{

HM::HM()
{
	ActionBase();

	m_pCtrl = NULL;

	m_motorPwmL = 0;
	m_motorPwmR = 0;
	m_motorPwmW = 0;
	m_bSpeaker = 0;

	m_maxSpeed = 100;
	m_ctrlB0 = 0;
	m_ctrlB1 = 0;
}

HM::~HM()
{
}

bool HM::init(Config* pConfig, AUTOPILOT_CONTROL* pAC)
{
	CHECK_F(this->ActionBase::init(pConfig, pAC) == false)
	m_pCtrl = pAC;

	F_INFO(pConfig->v("maxSpeed", &m_maxSpeed));
	F_INFO(pConfig->v("bSpeaker", &m_bSpeaker));
	F_INFO(pConfig->v("motorPwmL", &m_motorPwmL));
	F_INFO(pConfig->v("motorPwmR", &m_motorPwmR));
	F_INFO(pConfig->v("motorPwmW", &m_motorPwmW));


//link ROI tracker
//	string roiName = "";
//	F_ERROR_F(pConfig->v("ROItracker", &roiName));
//	m_pROITracker = (_ROITracker*) (pConfig->root()->getChildInstByName(&roiName));

	pConfig->m_pInst = this;

	return true;
}

void HM::update(void)
{
	this->ActionBase::update();

	NULL_(m_pCtrl);
	NULL_(m_pCtrl->m_pCAN);

	updateMotion();

}

void HM::updateMotion(void)
{

	updateCAN();
}

void HM::updateCAN(void)
{
	unsigned long addr = 0x113;
	unsigned char cmd[8];
	const static int dint = sizeof(int)*8-1;

	m_ctrlB0 = 0;
	m_ctrlB0 |= ((m_motorPwmR >> dint) << 4);
	m_ctrlB0 |= ((m_motorPwmL >> dint) << 5);
	m_ctrlB0 |= ((m_motorPwmW >> dint) << 6);

	m_motorPwmL = abs(confineVal(m_motorPwmL, m_maxSpeed, -m_maxSpeed));
	m_motorPwmR = abs(confineVal(m_motorPwmR, m_maxSpeed, -m_maxSpeed));
	m_motorPwmW = abs(confineVal(m_motorPwmW, m_maxSpeed, -m_maxSpeed));

	m_ctrlB1 = 0;
	m_ctrlB1 |= 1;						//tracktion motor relay
	m_ctrlB1 |= (1 << 1);				//working motor relay
	m_ctrlB1 |= (m_bSpeaker)?(1 << 5):0;//speaker
	m_ctrlB1 |= (1 << 6);				//external command
	m_ctrlB1 |= (1 << 7);				//0:rpm, 1:pwm

	int bFilter = 0x000f;

	cmd[0] = m_ctrlB0;
	cmd[1] = m_ctrlB1;
	cmd[2] = m_motorPwmL & bFilter;
	cmd[3] = (m_motorPwmL>>8) & bFilter;
	cmd[4] = m_motorPwmL & bFilter;
	cmd[5] = (m_motorPwmL>>8) & bFilter;
	cmd[6] = m_motorPwmW & bFilter;
	cmd[7] = (m_motorPwmW>>8) & bFilter;

	// send data:  id = 0x00, standrad frame, data len = 8, stmp: data buf
	// CAN.sendMsgBuf(0x113, 0, 8, cmd);

	m_pCtrl->m_pCAN->send(addr, 8, cmd);

//	cmd[0] = m_ctrlB0;
//	cmd[1] = m_ctrlB1;
//	cmd[2] = 0x40;
//	cmd[3] = 0x1f;
//	cmd[4] = 0x40;
//	cmd[5] = 0x1f;
//	cmd[6] = 0xc4;
//	cmd[7] = 0x09; //pwmM;


}

bool HM::draw(Frame* pFrame, iVec4* pTextPos)
{
	NULL_F(pFrame);
	Mat* pMat = pFrame->getCMat();

	putText(*pMat, "HM", cv::Point(pTextPos->m_x, pTextPos->m_y),
			FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);

	pTextPos->m_y += pTextPos->m_w;

	return true;
}

}
