#include "../ArduPilot/_AP_land.h"

namespace kai
{

_AP_land::_AP_land()
{
	m_pAP = NULL;
	m_pIRlock = NULL;

	m_altLandMode = 3.0;
	m_dTarget = -1.0;
	m_dHdg = 0.0;
	m_dzHdg = 360;
	m_targetType = landTarget_unknown;
}

_AP_land::~_AP_land()
{
}

bool _AP_land::init(void* pKiss)
{
	IF_F(!this->_AP_follow::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("altLandMode", &m_altLandMode);
	pK->v("dzHdg", &m_dzHdg);

	int wLen = 3;
	pK->v("wLen",&wLen);
	m_filter.init(wLen,2);

	string iName;
	iName = "";
	pK->v("_IRLock", &iName);
	m_pIRlock = (_DetectorBase*) (pK->root()->getChildInst(iName));

	return true;
}

bool _AP_land::start(void)
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

int _AP_land::check(void)
{
	return this->_AP_posCtrl::check();
}

void _AP_land::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		this->_AP_posCtrl::update();
		if(updateTarget())
		{
			updateCtrl();
		}
		else
		{
			m_filter.reset();
			m_dTarget = -1.0;
			m_bTarget = false;
			releaseCtrl();
		}
			
		this->autoFPSto();
	}
}

bool _AP_land::updateTarget(void)
{
	IF_F(check()<0);
	IF_F(!bActive());

	if(m_apMount.m_bEnable)
		m_pAP->setMount(m_apMount);


	m_bTarget = findTarget();
	IF_F(!m_bTarget);

	if(m_dTarget > 0.0 && m_dTarget < m_altLandMode)
	{
//		m_pAP->setApMode(LAND);
//		if(LANDED)
//		{
//			m_pMC->transit("RELEASE");
//		}

		m_pMC->getCurrentMission()->complete();
		return false;
	}


	m_vP.x = m_vTargetBB.midX();
	m_vP.y = m_vTargetBB.midY();
	m_vP.z = m_vTargetP.z;

	if(abs(m_dHdg) > m_dzHdg)
	{
		if(m_dHdg > 0.0)
			m_vP.w = 1.0;
		else
			m_vP.w = -1.0;
	}
	else
	{
		m_vP.w = m_vTargetP.w;
	}

	return true;
}

bool _AP_land::findTarget(void)
{
	IF_F(check()<0);

	OBJECT* tO = NULL;

	if(m_pDet)
	{
		OBJECT* pO;
		float topProb = 0.0;
		int i=0;
		while((pO = m_pDet->at(i++)) != NULL)
		{
			IF_CONT(pO->m_topClass != m_iClass);
			IF_CONT(pO->m_topProb < topProb);

			tO = pO;
			topProb = pO->m_topProb;
		}

		if(tO)
		{
			m_vTargetBB = tO->m_bb;
			m_filter.input(tO->m_dist);
			m_dTarget = m_filter.v();
			m_dHdg = dHdg<float>(0.0, tO->m_angle);
			m_targetType = landTarget_det;
			return true;
		}
	}

	NULL_F(m_pIRlock);

	tO = m_pIRlock->at(0);
	NULL_F(tO);

	m_vTargetBB = tO->m_bb;
	m_filter.input(tO->m_dist);
	m_dTarget = m_filter.v();
	m_dHdg = 0.0;
	m_targetType = landTarget_IR;

	return true;
}

void _AP_land::draw(void)
{
	this->_AP_follow::draw();
	IF_(check()<0);

	if (!m_bTarget)
	{
		addMsg("Target not found", 1);
		return;
	}

	if(m_targetType == landTarget_IR)
		addMsg("IR locked", 1);
	else
		addMsg("Tag locked", 1);

	vFloat2 c = m_vTargetBB.center();
	addMsg("Pos=("+f2str(c.x) + ", " +f2str(c.y)+ "), d=" + f2str(m_dTarget) + ", dHdg=" +f2str(m_dHdg), 1);

	IF_(!checkWindow());
	Mat* pMat = ((Window*) this->m_pWindow)->getFrame()->m();
}

}
