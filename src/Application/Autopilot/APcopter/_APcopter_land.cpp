#include "_APcopter_land.h"

namespace kai
{

_APcopter_land::_APcopter_land()
{
	m_pAP = NULL;
	m_pIRlock = NULL;
}

_APcopter_land::~_APcopter_land()
{
}

bool _APcopter_land::init(void* pKiss)
{
	IF_F(!this->_APcopter_follow::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	string iName;
	iName = "";
	pK->v("APcopter_base", &iName);
	m_pAP = (_APcopter_base*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pAP, iName + ": not found");

	iName = "";
	pK->v("_IRLock", &iName);
	m_pIRlock = (_DetectorBase*) (pK->root()->getChildInst(iName));

	return true;
}

bool _APcopter_land::start(void)
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

int _APcopter_land::check(void)
{
	NULL__(m_pAP,-1);

	return this->_APcopter_follow::check();
}

void _APcopter_land::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		this->_APcopter_posCtrl::update();
		if(updateLand())
		{
			updateCtrl();
		}
		else
		{
			releaseCtrl();
		}
			
		this->autoFPSto();
	}
}

bool _APcopter_land::updateLand(void)
{
	IF_F(check()<0);
	if(!bActive())
	{
		m_bTarget = false;
		if(m_pT)
			m_pT->stopTrack();

		return false;
	}

	int apMode = m_pAP->getApMode();
//	int apMissionSeq = m_pAP->m_pMavlink->m_mavMsg.mission_current.seq;

	if(m_pAP->getApMode() == LAND)
	{
		m_pAP->setApMode(GUIDED);
		return;
	}

	if(m_pAP->getApMode() != GUIDED)
	{
		releaseCtrl();
		m_tO.init();
		return;
	}

	if(m_apMount.m_bEnable)
		m_pAP->setMount(m_apMount);

	m_bTarget = updateTarget();
	if(m_pT)
	{
		if(m_bTarget)
			m_pT->startTrack(m_vTargetBB);

		if(m_pT->trackState() == track_update)
		{
			m_vTargetBB = *m_pT->getBB();
			m_bTarget = true;
		}
	}

	IF_F(!m_bTarget);

	m_vP.x = m_vTargetBB.midX();
	m_vP.y = m_vTargetBB.midY();
	m_vP.z = m_vTargetP.z;
	m_vP.w = m_vTargetP.w;

	return true;
}

bool _APcopter_land::updateTarget(void)
{
	IF_F(check()<0);

	OBJECT* pO;
	OBJECT* tO = NULL;
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
		return true;
	}

	NULL_F(m_pIRlock);

	tO = m_pIRlock->at(0);
	NULL_F(tO);


	m_tO = *pO;
	vFloat2 vTarget;

	if(m_tO.m_topClass == INT_MAX)
	{
		//IR
		vTarget = m_tO.m_bb.center();
	}
	else
	{
		//Tag
		vTarget = m_tO.m_c;
	}





	return true;
}

void _APcopter_land::draw(void)
{
	this->_APcopter_follow::draw();
	IF_(check()<0);

	addMsg(
			"Set target: V = (" + f2str(m_spt.vx,7) + ", " + f2str(m_spt.vy,7)
					+ ", " + f2str(m_spt.vz,7) + "), P = (" + f2str(m_spt.x,7)
					+ ", " + f2str(m_spt.y,7) + ", " + f2str(m_spt.z,7) + ")",1);

	if (m_tO.m_topClass < 0)
	{
		addMsg("Target not found");
	}
	else if(m_tO.m_topClass == INT_MAX)
	{
		addMsg("IR locked");
		vFloat2 c = m_tO.m_bb.center();
		addMsg("("+f2str(c.x) + ", " +f2str(c.y)+")");
	}
	else
	{
		addMsg("Tag locked = " + i2str(m_tO.m_topClass));
		addMsg("("+f2str(m_tO.m_c.x) + ", " +f2str(m_tO.m_c.y)+")");
	}

	IF_(!checkWindow());
	Mat* pMat = ((Window*) this->m_pWindow)->getFrame()->m();
}

}
