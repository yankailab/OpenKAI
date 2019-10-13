#include "_Rover_base.h"

namespace kai
{

_Rover_base::_Rover_base()
{
	m_pCMD = NULL;
	m_pMavlink = NULL;
	m_pPIDhdg = NULL;

	m_ctrl.init();
	m_pCtrl = (void*)&m_ctrl;
}

_Rover_base::~_Rover_base()
{
}

bool _Rover_base::init(void* pKiss)
{
	IF_F(!this->_AutopilotBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

	Kiss** ppD = pK->getChildItr();
	int i = 0;
	while (ppD[i])
	{
		Kiss* pD = ppD[i++];

		ROVER_DRIVE d;
		d.init();
		pD->v("kDir",&d.m_kDir);
		pD->v("sDir",&d.m_sDir);
		pD->v("iPWM",&d.m_iPWM);
		pD->v("pwmH",&d.m_pwmH);
		pD->v("pwmM",&d.m_pwmM);
		pD->v("pwmL",&d.m_pwmL);

		m_ctrl.m_vDrive.push_back(d);
	}

	string iName;
	iName = "";
	F_ERROR_F(pK->v("_RoverCMD", &iName));
	m_pCMD = (_Rover_CMD*) (pK->root()->getChildInst(iName));
	NULL_Fl(m_pCMD, iName+": not found");

	iName = "";
	F_ERROR_F(pK->v("_Mavlink", &iName));
	m_pMavlink = (_Mavlink*) (pK->root()->getChildInst(iName));
	NULL_Fl(m_pMavlink, iName+": not found");

	iName = "";
	pK->v("PIDhdg", &iName);
	m_pPIDhdg = (PIDctrl*) (pK->root()->getChildInst(iName));
	NULL_Fl(m_pPIDhdg, iName+": not found");

	return true;
}

int _Rover_base::check(void)
{
	NULL__(m_pCMD, -1);
	NULL__(m_pMavlink, -1);
	NULL__(m_pPIDhdg, -1);

	return 0;
}

void _Rover_base::update(void)
{
	this->_AutopilotBase::update();
	IF_(check()<0);

	//hard switch priority
	if(m_pCMD->m_mode == rover_idle || m_pCMD->m_mode == rover_manual)
	{
		m_pMC->transit("IDLE");
		setSpeed(0.0);
	}

	//sensor
	if(m_tStamp - m_pMavlink->m_mavMsg.time_stamps.global_position_int > USEC_1SEC)
	{
		m_pMavlink->requestDataStream(MAV_DATA_STREAM_POSITION, 5);
		return;
	}

	uint16_t h = m_pMavlink->m_mavMsg.global_position_int.hdg;
	if(h < UINT16_MAX)
	{
		m_ctrl.m_hdg = (float)h * 1e-2;
	}

}

void _Rover_base::setSpeed(float nSpeed)
{
	m_ctrl.m_nSpeed = constrain<float>(nSpeed, -1.0, 1.0);
}

void _Rover_base::setPinout(uint8_t pin, uint8_t status)
{
	NULL_(m_pCMD);
	m_pCMD->pinOut(pin, status);
}

void _Rover_base::draw(void)
{
	this->_AutopilotBase::draw();

//	string msg = *this->getName()
//			+ ": mode=" + c_roverModeName[m_pCMD->m_mode]
//			+ ", hdg=" + f2str(m_hdg)
//			+ ", targetHdg=" + f2str(m_targetHdg)
//			+ ", nSpeed=" + f2str(m_nSpeed);
//	addMsg(msg);
//
//	for(int i=0;i<m_vPWM.size();i++)
//	{
//		ROVER_PWM_CHANNEL* pM = &m_vPWM[i];
//		msg = "PWM" + i2str(i)
//				+ ": H=" + i2str(pM->m_H)
//				+ ", M=" + i2str(pM->m_M)
//				+ ", L=" + i2str(pM->m_L)
//				+ ", pwm=" + i2str(pM->m_pwm)
//				;
//		addMsg(msg,1);
//	}
}

}
