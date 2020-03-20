#include "../ArduPilot/_AP_actuator.h"

namespace kai
{

_AP_actuator::_AP_actuator()
{
	m_pMav = NULL;
	m_pRCmode = NULL;
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

	string iName;
	iName = "";
	pK->v("_Mavlink", &iName);
	m_pMav = (_Mavlink*) (pK->root()->getChildInst(iName));
	NULL_Fl(m_pMav, "_Mavlink: " + iName + " not found");

	int iRC = -1;
	pK->v("iRCmode", &iRC);
	m_pRCmode = m_pMav->getRCinRaw(iRC);
	NULL_Fl(m_pRCmode, "iRCmode not correct");

	pK->v("vPWMrange", &m_vPWMrange);


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
		a.m_ppRCin[0] = m_pMav->getRCinRaw(vRCin.x);
		a.m_ppRCin[1] = m_pMav->getRCinRaw(vRCin.y);
		a.m_ppRCin[2] = m_pMav->getRCinRaw(vRCin.z);
		a.m_ppRCin[3] = m_pMav->getRCinRaw(vRCin.w);

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
	NULL__(m_pMav, -1);

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
	NULL_(m_pRCmode);
	if(*m_pRCmode < m_vPWMrange.x || *m_pRCmode > m_vPWMrange.w)
	{
		m_iMode = -1;
		return;
	}

	if(*m_pRCmode < m_vPWMrange.y)
		m_iMode = 0;
	else if(*m_pRCmode < m_vPWMrange.z)
		m_iMode = 1;
	else
		m_iMode = 2;

	for (int i=0; i<m_vActuator.size(); i++)
	{
		AP_actuator* pA = &m_vActuator[i];
		IF_CONT(pA->m_iMode != m_iMode);
		pA->update();
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
