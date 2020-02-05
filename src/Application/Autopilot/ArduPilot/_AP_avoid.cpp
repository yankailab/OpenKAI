#include "../ArduPilot/_AP_avoid.h"

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
	IF_F(!this->_AutopilotBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	string iName;
	iName = "";
	F_INFO(pK->v("APcopter_base", &iName));
	m_pAP = (_AP_base*) (pK->parent()->getChildInst(iName));

	iName = "";
	F_ERROR_F(pK->v("_Mavlink", &iName));
	m_pMavlink = (_Mavlink*) (pK->root()->getChildInst(iName));
	NULL_Fl(m_pMavlink, iName+": not found");

	iName = "";
	F_ERROR_F(pK->v("_DetectorBase", &iName));
	m_pDet = (_DetectorBase*) (pK->root()->getChildInst(iName));
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

		this->_AutopilotBase::update();
		updateTarget();

		this->autoFPSto();
	}
}

void _AP_avoid::updateTarget(void)
{
	IF_(check()<0);

	OBJECT o;
	o.init();
	OBJECT* pO;
	int i=0;
	while((pO = m_pDet->at(i++)) != NULL)
	{
		o = *pO;
		o.m_topClass = 0;
	}

	if(o.m_topClass<0)
	{
		m_obs.init();
		LOG_I("Target not found");
		return;
	}

	m_obs = o;

	if(m_pAP->m_apMode == LOITER)
	{
		LOG_I("Already Loiter");
		return;
	}

	m_pAP->setApMode(LOITER);
	LOG_I("Set Loiter");
}

void _AP_avoid::draw(void)
{
	this->_AutopilotBase::draw();
	IF_(check()<0);

	string msg = "nTarget=" + i2str(m_pDet->size());
	addMsg(msg);

	IF_(!checkWindow());
	Mat* pMat = ((Window*) this->m_pWindow)->getFrame()->m();

	IF_(m_obs.m_topClass<0);

	vInt2 cs;
	cs.x = pMat->cols;
	cs.y = pMat->rows;
	Rect r = convertBB<vInt4>(convertBB(m_obs.m_bb, cs));
	rectangle(*pMat, r, Scalar(0,0,255), 3);

}

}
