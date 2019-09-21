#include "_APcopter_follow.h"

namespace kai
{

_APcopter_follow::_APcopter_follow()
{
	m_pAP = NULL;
	m_pPC = NULL;
	m_pDet = NULL;

	m_iClass = -1;
	m_apMode = -1;
	m_tO.init();

	m_vTargetP.init();
	m_vTargetP.x = 0.5;
	m_vTargetP.y = 0.5;
	m_vP.init();
	m_apMount.init();

	m_bTracker = false;
	m_pT = NULL;
}

_APcopter_follow::~_APcopter_follow()
{
}

bool _APcopter_follow::init(void* pKiss)
{
	IF_F(!this->_ActionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("iClass",&m_iClass);
	pK->v("bTracker",&m_bTracker);
	pK->v("apMode",&m_apMode);

	Kiss* pG = pK->o("mount");
	if(!pG->empty())
	{
		pG->v("bEnable", &m_apMount.m_bEnable);

		float p=0, r=0, y=0;
		pG->v("pitch", &p);
		pG->v("roll", &r);
		pG->v("yaw", &y);

		m_apMount.m_control.input_a = p * 100;	//pitch
		m_apMount.m_control.input_b = r * 100;	//roll
		m_apMount.m_control.input_c = y * 100;	//yaw
		m_apMount.m_control.save_position = 0;

		pG->v("stabPitch", &m_apMount.m_config.stab_pitch);
		pG->v("stabRoll", &m_apMount.m_config.stab_roll);
		pG->v("stabYaw", &m_apMount.m_config.stab_yaw);
		pG->v("mountMode", &m_apMount.m_config.mount_mode);
	}

	string iName;

	iName = "";
	pK->v("_APcopter_base", &iName);
	m_pAP = (_APcopter_base*) (pK->parent()->getChildInst(iName));
	IF_Fl(!m_pAP, iName + ": not found");

	iName = "";
	pK->v("_DetectorBase", &iName);
	m_pDet = (_DetectorBase*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pDet, iName + ": not found");

	iName = "";
	pK->v("_APcopter_posCtrl", &iName);
	m_pPC = (_APcopter_posCtrl*) (pK->parent()->getChildInst(iName));
	IF_Fl(!m_pPC, iName + ": not found");

	iName = "";
	pK->v("_TrackerBase", &iName);
	m_pT = (_TrackerBase*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pT, iName + ": not found");

	pK->v("vP",&m_vP);
	pK->v("vTargetP",&m_vTargetP);

	m_pPC->setPos(m_vP);
	m_pPC->setTargetPos(m_vTargetP);

	return true;
}

bool _APcopter_follow::start(void)
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

int _APcopter_follow::check(void)
{
	NULL__(m_pAP,-1);
	NULL__(m_pPC,-1);
	NULL__(m_pDet,-1);

	if(m_bTracker)
	{
		NULL__(m_pT,-1);
	}

	return this->_ActionBase::check();
}

void _APcopter_follow::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		this->_ActionBase::update();
		updateTarget();

		this->autoFPSto();
	}
}

void _APcopter_follow::updateTarget(void)
{
	IF_(check()<0);
	if(!bActive())
	{
		m_vP = m_vTargetP;
		m_pPC->setEnable(false);
		m_pDet->goSleep();
		if(m_bTracker)
		{
			m_pT->stopTrack();
		}

		return;
	}

	if(m_bMissionChanged)
	{
		m_pDet->wakeUp();
	}

	if(m_apMount.m_bEnable)
	{
		m_pAP->setMount(m_apMount);
	}

	if(!find())
	{
		m_vP = m_vTargetP;
		m_pPC->setEnable(false);
		return;
	}

	m_pPC->setPos(m_vP);
	m_pPC->setEnable(true);
}

bool _APcopter_follow::find(void)
{
	IF__(check()<0, false);

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

	vFloat4 bb;
	if(m_bTracker)
	{
		if(tO)
		{
			m_pT->startTrack(tO->m_bb);
			bb = tO->m_bb;
		}
		else
		{
			IF_F(m_pT->trackState() != track_update);
			bb = *m_pT->getBB();
		}
	}
	else
	{
		NULL_F(tO);
		bb = tO->m_bb;
	}

	m_vP.x = bb.midX();
	m_vP.y = bb.midY();
	m_vP.z = tO->m_dist;

	return true;
}

bool _APcopter_follow::draw(void)
{
	IF_F(!this->_ActionBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();
	IF_F(pMat->empty());
	IF_F(check()<0);

	pWin->tabNext();

	if(!bActive())
		pWin->addMsg("Inactive");

	pWin->addMsg("vTargetP = (" + f2str(m_vTargetP.x) + ", "
							   + f2str(m_vTargetP.y) + ", "
					           + f2str(m_vTargetP.z) + ", "
				           	   + f2str(m_vTargetP.w) + ")");

	pWin->addMsg("vP = (" + f2str(m_vP.x) + ", "
							   + f2str(m_vP.y) + ", "
					           + f2str(m_vP.z) + ", "
				           	   + f2str(m_vP.w) + ")");

	pWin->tabPrev();

	return true;
}

bool _APcopter_follow::console(int& iY)
{
	IF_F(!this->_ActionBase::console(iY));
	IF_F(check()<0);

	string msg;

	if(!bActive())
	{
		C_MSG("Inactive");
	}

	C_MSG("vTarget = (" + f2str(m_vTargetP.x) + ", "
				     	 + f2str(m_vTargetP.y) + ", "
						 + f2str(m_vTargetP.z) + ", "
						 + f2str(m_vTargetP.w) + ")");

	C_MSG("vP = (" + f2str(m_vP.x) + ", "
							   + f2str(m_vP.y) + ", "
					           + f2str(m_vP.z) + ", "
				           	   + f2str(m_vP.w) + ")");

	return true;
}

}
