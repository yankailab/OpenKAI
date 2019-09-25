#include "_APcopter_follow.h"

namespace kai
{

_APcopter_follow::_APcopter_follow()
{
	m_pAP = NULL;
	m_pAL = NULL;
	m_pPC = NULL;
	m_pDet = NULL;

	m_tO.init();
	m_iClass = -1;
	m_timeOut = 100000;

	m_vBB.init(-1.0);
	m_vP.init();
	m_vP.x = 0.5;
	m_vP.y = 0.5;
	m_vTargetOrigin.init();
	m_vTargetOrigin.x = 0.5;
	m_vTargetOrigin.y = 0.5;
	m_vTargetP = m_vTargetOrigin;
	m_vKtarget.init();
	m_vTargetPregion.x = 0.2;
	m_vTargetPregion.y = 0.8;

	m_apMount.init();
	m_pT = NULL;

	m_bBB = false;

	m_ieSend.init(100000);
}

_APcopter_follow::~_APcopter_follow()
{
}

bool _APcopter_follow::init(void* pKiss)
{
	IF_F(!this->_ActionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("iClass",&m_iClass);
	pK->v("timeOut",&m_timeOut);
	pK->v("tIntSend",&m_ieSend.m_tInterval);
	pK->v("vTargetOrigin",&m_vTargetOrigin);
	pK->v("vKtarget",&m_vKtarget);
	pK->v("vTargetPregion",&m_vTargetPregion);

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
	m_pAP = (_APcopter_base*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pAP, iName + ": not found");

	iName = "";
	pK->v("_APcopter_link", &iName);
	m_pAL = (_APcopter_link*) (pK->root()->getChildInst(iName));
	NULL_Fl(m_pAL, iName+": not found");

	iName = "";
	pK->v("_APcopter_posCtrl", &iName);
	m_pPC = (_APcopter_posCtrl*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pPC, iName + ": not found");

	iName = "";
	pK->v("_TrackerBase", &iName);
	m_pT = (_TrackerBase*) (pK->root()->getChildInst(iName));

	iName = "";
	pK->v("_DetectorBase", &iName);
	m_pDet = (_DetectorBase*) (pK->root()->getChildInst(iName));

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
	NULL__(m_pAL,-1);
	NULL__(m_pPC,-1);

	return this->_ActionBase::check();
}

void _APcopter_follow::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		this->_ActionBase::update();

		updateTargetPos();
		updatePos();
		sendClient();

		this->autoFPSto();
	}
}

void _APcopter_follow::updateTargetPos(void)
{
	IF_(check()<0);

	m_vTargetP.x = constrain<float>(m_vTargetOrigin.x + m_vKtarget.x * (float)m_pAP->m_vLocalPos.y, m_vTargetPregion.x, m_vTargetPregion.y);
	m_vTargetP.y = constrain<float>(m_vTargetOrigin.y + m_vKtarget.y * (float)m_pAP->m_vLocalPos.x, m_vTargetPregion.x, m_vTargetPregion.y);
	m_pPC->setTargetPos(m_vTargetP);

}

void _APcopter_follow::updatePos(void)
{
	IF_(check()<0);
	if(!bActive())
	{
		m_vP = m_vTargetP;
		m_pPC->setEnable(false);
		if(m_pT)
		{
			m_pT->stopTrack();
		}

		return;
	}

	if(m_apMount.m_bEnable)
	{
		m_pAP->setMount(m_apMount);
	}


	m_bBB = updateBBclient(&m_vBB);

	if(!m_bBB && m_pAL->m_iState==APFOLLOW_ON)
	{
		m_bBB = updateBBdet(&m_vBB);
	}

	if(m_pT && m_pAL->m_iState==APFOLLOW_ON)
	{
		if(m_bBB)
			m_pT->startTrack(m_vBB);

		if(m_pT->trackState() == track_update)
		{
			m_vBB = *m_pT->getBB();
			m_bBB = true;
		}
	}

	if(m_bBB)
	{
		m_vP.x = m_vBB.midX();
		m_vP.y = m_vBB.midY();
	}
	else
	{
		m_vP.x = m_vTargetP.x;
		m_vP.y = m_vTargetP.y;
	}


	bool bAlt = false;
	if(m_tStamp - m_pAL->m_tAlt < m_timeOut)
	{
		m_vP.z = m_pAL->m_alt;
		bAlt = true;
	}
	else
	{
		m_vP.z = m_vTargetP.z;
	}


	bool bHdg = false;
	if(m_tStamp - m_pAL->m_tHdg < m_timeOut)
	{
		m_vP.w = m_pAL->m_hdg;
		bHdg = true;
	}
	else
	{
		m_vP.w = m_vTargetP.w;
	}


	if(m_bBB || bAlt || bHdg)
	{
		m_pPC->setPos(m_vP);
		m_pPC->setEnable(true);
		return;
	}

	m_vP = m_vTargetP;
	m_pPC->setPos(m_vP);
	m_pPC->setEnable(false);
}

bool _APcopter_follow::updateBBclient(vFloat4* pBB)
{
	IF__(check()<0, false);
	NULL_F(pBB);
	IF__(m_tStamp - m_pAL->m_tBB > m_timeOut, false);

	*pBB = m_pAL->m_vBB;
	return true;
}

bool _APcopter_follow::updateBBdet(vFloat4* pBB)
{
	IF_F(check()<0);
	NULL_F(m_pDet);
	NULL_F(pBB);

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

	NULL_F(tO);

	*pBB = tO->m_bb;
	return true;
}

void _APcopter_follow::sendClient(void)
{
	IF_(check()<0);
	IF_(!m_ieSend.update(m_tStamp));

	m_pAL->state(m_pAL->m_iState);

	if(m_bBB)
		m_pAL->setBB(m_vBB);

	float w = m_vBB.width()*0.5;
	float h = m_vBB.height()*0.5;

	vFloat4 tBB;
	tBB.x = m_vTargetP.x - w;
	tBB.y = m_vTargetP.y - h;
	tBB.z = m_vTargetP.x + w;
	tBB.w = m_vTargetP.y + h;
	m_pAL->setTargetBB(tBB);

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
