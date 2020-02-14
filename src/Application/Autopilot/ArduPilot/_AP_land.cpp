#include "../ArduPilot/_AP_land.h"

namespace kai
{

_AP_land::_AP_land()
{
	m_pAPtarget = NULL;
	m_pIRlock = NULL;

	m_altLandMode = 3.0;
	m_detSizeLandMode = 0.25;
	m_vRoiDetDescent.init(0.0, 0.0, 1.0, 1.0);
	m_dTarget = -1.0;
	m_dHdg = 0.0;
	m_dzHdg = 360;
	m_detRdz = 1.0;
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
	pK->v("detSizeLandMode", &m_detSizeLandMode);
	pK->v("dzHdg", &m_dzHdg);
	pK->v("vRoiDetDescent", &m_vRoiDetDescent);
	pK->v("detRdz", &m_detRdz);

	int wLen = 3;
	pK->v("wLen",&wLen);
	m_filter.init(wLen,2);

	string iName;
	iName = "";
	pK->v("_IRLock", &iName);
	m_pIRlock = (_DetectorBase*) (pK->root()->getChildInst(iName));

	iName = "";
	pK->v("_AP_target_base", &iName);
	m_pAPtarget = (_AP_base*) (pK->parent()->getChildInst(iName));

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

		if(!updateTarget())
		{
			m_filter.reset();
			m_dTarget = -1.0;
			m_bTarget = false;
			m_targetType = landTarget_unknown;
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


	m_bTarget = findTargetLocal();
	if(m_bTarget)
	{
		float w = m_vTargetBB.width();
		float h = m_vTargetBB.height();

		if(m_dTarget > 0.0 &&
			m_dTarget < m_altLandMode &&
			w > m_detSizeLandMode &&
			h > m_detSizeLandMode
			)
		{
			m_pMC->getCurrentMission()->complete();
			return false;
		}

		m_vP.x = m_vTargetBB.midX();
		m_vP.y = m_vTargetBB.midY();

		if(m_vP.x > m_vRoiDetDescent.x &&
			m_vP.x < m_vRoiDetDescent.z &&
			m_vP.y > m_vRoiDetDescent.y &&
			m_vP.y < m_vRoiDetDescent.w
			)
		{
			m_vP.z = m_vTargetP.z;
		}
		else
		{
			m_vP.z = 0.0;
		}

		if(abs(m_dHdg) > m_dzHdg)
		{
			if(m_dHdg > 0.0)
				m_vP.w = m_vTargetP.w;
			else
				m_vP.w = -m_vTargetP.w;
		}
		else
		{
			m_vP.w = 0.0;
		}

		setPosLocal();
		return true;
	}

	m_bTarget = findTargetGlobal();
	if(m_bTarget)
	{
		setPosGlobal();
		return true;
	}

	return false;
}

bool _AP_land::findTargetLocal(void)
{
	IF_F(check()<0);

	OBJECT* tO = NULL;

	if(m_pDet)
	{
		OBJECT* pO;
		float minR = FLT_MAX;
		float topProb = 0.0;
		int i=0;
		while((pO = m_pDet->at(i++)) != NULL)
		{
//			IF_CONT(pO->m_topClass != m_iClass);
//			IF_CONT(pO->m_topProb < topProb);
			IF_CONT(pO->m_r > minR);

			tO = pO;
			minR = pO->m_r * m_detRdz;
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
	m_filter.reset();
	m_dTarget = -1.0;
	m_dHdg = 0.0;
	m_targetType = landTarget_IR;
	return true;
}

bool _AP_land::findTargetGlobal(void)
{
	IF_F(check()<0);
	NULL_F(m_pAPtarget);
	NULL_F(m_pAPtarget->m_pMavlink);
	IF_F(m_pAPtarget->m_pMavlink->m_mavMsg.time_stamps.global_position_int <= 0);

	vDouble3 apPos = m_pAPtarget->getGlobalPos();
	IF_F(EAQ(apPos.x, 0.0, 1e-7));
	IF_F(EAQ(apPos.y, 0.0, 1e-7));

	m_vTargetGlobal.x = apPos.x;
	m_vTargetGlobal.y = apPos.y;
	m_targetType = landTarget_global;

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
		addMsg("Target Type: IR locked", 1);
	else if(m_targetType == landTarget_det)
		addMsg("Target Type: Tag locked", 1);
	else if(m_targetType == landTarget_global)
		addMsg("Target Type: Global pos", 1);
	else
		addMsg("Target Type Unknown");

	addMsg("Local Target");
	vFloat2 c = m_vTargetBB.center();
	addMsg("Pos=("+f2str(c.x) + ", " +f2str(c.y)+ "), d=" + f2str(m_dTarget) + ", dHdg=" + f2str(m_dHdg), 1);
	addMsg("Size=(" + f2str(m_vTargetBB.x) + ", " + f2str(m_vTargetBB.y) + ")", 1);

	IF_(!checkWindow());
	Mat* pMat = ((Window*) this->m_pWindow)->getFrame()->m();
}

}
