#include "../ArduPilot/_AP_avoid.h"

#ifdef USE_OPENCV

namespace kai
{

_AP_avoid::_AP_avoid()
{
	m_pAP = NULL;
	m_pDet = NULL;
	m_pMavlink = NULL;
}

_AP_avoid::~_AP_avoid()
{
}

bool _AP_avoid::init(void* pKiss)
{
	IF_F(!this->_StateBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	string n;
	n = "";
	F_INFO(pK->v("APcopter_base", &n));
	m_pAP = (_AP_base*)pK->getInst(n);

	n = "";
	F_ERROR_F(pK->v("_Mavlink", &n));
	m_pMavlink = (_Mavlink*)pK->getInst(n);
	NULL_Fl(m_pMavlink, n+": not found");

	n = "";
	F_ERROR_F(pK->v("_DetectorBase", &n));
	m_pDet = (_DetectorBase*)pK->getInst(n);
	NULL_Fl(m_pDet, n+": not found");

	return true;
}

bool _AP_avoid::start(void)
{
    NULL_F(m_pT);
	return m_pT->start(getUpdate, this);
}

int _AP_avoid::check(void)
{
	NULL__(m_pAP, -1);
	NULL__(m_pDet, -1);
	NULL__(m_pMavlink, -1);

	return this->_StateBase::check();
}

void _AP_avoid::update(void)
{
	while(m_pT->bRun())
	{
		m_pT->autoFPSfrom();

		this->_StateBase::update();
		updateTarget();

		m_pT->autoFPSto();
	}
}

void _AP_avoid::updateTarget(void)
{
	IF_(check()<0);

	_Object o;
	o.init();
	_Object* pO;
	int i=0;
	while((pO = m_pDet->m_pU->get(i++)) != NULL)
	{
		o = *pO;
		o.setTopClass(0);
	}

	if(o.getTopClass()<0)
	{
		m_obs.init();
		LOG_I("Target not found");
		return;
	}

	m_obs = o;

	if(m_pAP->m_apMode == AP_COPTER_LOITER)
	{
		LOG_I("Already Loiter");
		return;
	}

	m_pAP->setApMode(AP_COPTER_LOITER);
	LOG_I("Set Loiter");
}

void _AP_avoid::draw(void)
{
	this->_StateBase::draw();
	IF_(check()<0);

	string msg = "nTarget=" + i2str(m_pDet->m_pU->size());
	addMsg(msg);

	IF_(!checkWindow());
	Mat* pM = ((_WindowCV*) this->m_pWindow)->getFrame()->m();

	IF_(m_obs.getTopClass()<0);

	Rect r = bb2Rect(m_obs.getBB2Dscaled(pM->cols, pM->rows));
	rectangle(*pM, r, Scalar(0,0,255), 3);

}

}
#endif
