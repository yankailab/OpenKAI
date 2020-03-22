#include "../ArduPilot/_AP_actuator.h"

namespace kai
{

_AP_actuator::_AP_actuator()
{
	m_pAP = NULL;
	m_iRCmode = 0;
	m_iMode = -1;
	m_vPWMrange.init(900, 1250, 1750, 2100);

}

_AP_actuator::~_AP_actuator()
{
}

bool _AP_actuator::init(void* pKiss)
{
	IF_F(!this->_AutopilotBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("vPWMrange", &m_vPWMrange);
	pK->v("iRCmode", &m_iRCmode);

	string iName;

	Kiss** pItr = pK->getChildItr();
	int i = 0;
	while (pItr[i])
	{
		Kiss* pA = pItr[i++];
		AP_actuator a;
		a.init();

		iName = "";
		pA->v("_ActuatorBase", &iName);
		a.m_pA = (_ActuatorBase*) (pK->root()->getChildInst(iName));
		NULL_Fl(a.m_pA, "_ActuatorBase: " + iName + " not found");

		vInt4 vRCin;
		vRCin.init(-1);
		pA->v("vRCin", &vRCin);
		a.m_pRCin[0] = vRCin.x;
		a.m_pRCin[1] = vRCin.y;
		a.m_pRCin[2] = vRCin.z;
		a.m_pRCin[3] = vRCin.w;

		vFloat4 vF;
		if(pA->v("vInc", &vF))
		{
			a.m_pInc[0] = vF.x;
			a.m_pInc[1] = vF.y;
			a.m_pInc[2] = vF.z;
			a.m_pInc[3] = vF.w;
		}
		pA->v("vPWMrange", &a.m_vPWMrange);
		pA->v("vK", &a.m_vK);
		pA->v("vSpeed", &a.m_vSpeed);
		pA->v("iMode", &a.m_iMode);

		m_vActuator.push_back(a);
	}

	iName = "";
	pK->v("_AP_base", &iName);
	m_pAP = (_AP_base*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pAP, iName + ": not found");

	return true;
}

bool _AP_actuator::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		LOG(ERROR)<< "Return code: "<< retCode;
		m_bThreadON = false;
		return false;
	}

	return true;
}

int _AP_actuator::check(void)
{
	NULL__(m_pAP, -1);
	NULL__(m_pAP->m_pMav, -1);

	return this->_AutopilotBase::check();
}

void _AP_actuator::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		this->_AutopilotBase::update();
		updateActuator();

		this->autoFPSto();
	}
}

void _AP_actuator::updateActuator(void)
{
	IF_(check() < 0);
	IF_(!bActive());

	uint16_t rcMode = m_pAP->m_pMav->m_rcChannels.getRC(m_iRCmode);
	if(rcMode < m_vPWMrange.x || rcMode > m_vPWMrange.w)
	{
		m_iMode = -1;
		return;
	}

	if(rcMode < m_vPWMrange.y)
		m_iMode = 0;
	else if(rcMode < m_vPWMrange.z)
		m_iMode = 1;
	else
		m_iMode = 2;

	for (int i=0; i<m_vActuator.size(); i++)
	{
		AP_actuator* pA = &m_vActuator[i];
		IF_CONT(pA->m_iMode != m_iMode);
		pA->update(m_pAP->m_pMav);
	}
}

void _AP_actuator::draw(void)
{
	IF_(check() < 0);
	this->_AutopilotBase::draw();
	drawActive();

	addMsg("iMode: "+i2str(m_iMode), 1);
}

}
