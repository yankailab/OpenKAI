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

	string iName;
	iName = "";
	F_INFO(pK->v("APcopter_base", &iName));
	m_pAP = (_AP_base*)pK->getInst(iName);

	iName = "";
	F_ERROR_F(pK->v("_Mavlink", &iName));
	m_pMavlink = (_Mavlink*)pK->getInst(iName);
	NULL_Fl(m_pMavlink, iName+": not found");

	iName = "";
	F_ERROR_F(pK->v("_DetectorBase", &iName));
	m_pDet = (_DetectorBase*)pK->getInst(iName);
	NULL_Fl(m_pDet, iName+": not found");

	return true;
}

bool _AP_avoid::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		LOG(ERROR) << "Return code: "<< retCode;
		m_bThreadON = false;
		return false;
	}

	return true;
}

int _AP_avoid::check(void)
{
	NULL__(m_pAP, -1);
	NULL__(m_pDet, -1);
	NULL__(m_pMavlink, -1);

	return 0;
}

void _AP_avoid::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		this->_StateBase::update();
		updateTarget();

		this->autoFPSto();
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
	Mat* pM = ((Window*) this->m_pWindow)->getFrame()->m();

	IF_(m_obs.getTopClass()<0);

	Rect r = bb2Rect(m_obs.getBB2Dscaled(pM->cols, pM->rows));
	rectangle(*pM, r, Scalar(0,0,255), 3);

}

}
#endif
