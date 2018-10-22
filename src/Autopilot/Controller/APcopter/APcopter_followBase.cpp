#include "APcopter_followBase.h"

namespace kai
{

APcopter_followBase::APcopter_followBase()
{
	m_pAP = NULL;
	m_pDet = NULL;
	m_tStampDet = 0;
	m_iClass = -1;

	m_iTracker = 0;
	m_bUseTracker = false;
	m_pTracker[0] = NULL;
	m_pTracker[1] = NULL;
	m_pTnow = NULL;
	m_pTnew = NULL;

	m_vTarget.init();

	m_vGimbal.init();
	m_gimbalControl.input_a = m_vGimbal.x * 100;	//pitch
	m_gimbalControl.input_b = m_vGimbal.y * 100;	//roll
	m_gimbalControl.input_c = m_vGimbal.z * 100;	//yaw
	m_gimbalControl.save_position = 0;

	m_gimbalConfig.stab_pitch = 1;
	m_gimbalConfig.stab_roll = 1;
	m_gimbalConfig.stab_yaw = 1;
	m_gimbalConfig.mount_mode = 2;
}

APcopter_followBase::~APcopter_followBase()
{
}

bool APcopter_followBase::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK,iClass);
	KISSm(pK,bUseTracker);

	Kiss* pG = pK->o("gimbal");
	if(!pG->empty())
	{
		pG->v("pitch", &m_vGimbal.x);
		pG->v("roll", &m_vGimbal.y);
		pG->v("yaw", &m_vGimbal.z);

		m_gimbalControl.input_a = m_vGimbal.x * 100;	//pitch
		m_gimbalControl.input_b = m_vGimbal.y * 100;	//roll
		m_gimbalControl.input_c = m_vGimbal.z * 100;	//yaw
		m_gimbalControl.save_position = 0;

		pG->v("stabPitch", &m_gimbalConfig.stab_pitch);
		pG->v("stabRoll", &m_gimbalConfig.stab_roll);
		pG->v("stabYaw", &m_gimbalConfig.stab_yaw);
		pG->v("mountMode", &m_gimbalConfig.mount_mode);
	}

	//link
	string iName;

	if(m_bUseTracker)
	{
		iName = "";
		pK->v("_TrackerBase1", &iName);
		m_pTracker[0] = (_TrackerBase*) (pK->root()->getChildInst(iName));
		IF_Fl(!m_pTracker[0], iName + ": not found");

		iName = "";
		pK->v("_TrackerBase2", &iName);
		m_pTracker[1] = (_TrackerBase*) (pK->root()->getChildInst(iName));
		IF_Fl(!m_pTracker[1], iName + ": not found");

		m_pTnow = m_pTracker[m_iTracker];
		m_pTnew = m_pTracker[1-m_iTracker];
	}

	iName = "";
	pK->v("APcopter_base", &iName);
	m_pAP = (APcopter_base*) (pK->parent()->getChildInst(iName));
	IF_Fl(!m_pAP, iName + ": not found");

	iName = "";
	pK->v("_ObjectBase", &iName);
	m_pDet = (_ObjectBase*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pDet, iName + ": not found");

	return true;
}

int APcopter_followBase::check(void)
{
	NULL__(m_pAP,-1);
	NULL__(m_pAP->m_pMavlink,-1);
	NULL__(m_pDet,-1);
	_VisionBase* pV = m_pDet->m_pVision;
	NULL__(pV,-1);
	if(m_bUseTracker)
	{
		NULL__(m_pTracker[0],-1);
		NULL__(m_pTracker[1],-1);
	}

	return this->ActionBase::check();
}

bool APcopter_followBase::find(void)
{
	IF__(check()<0, false);

	//find target
	OBJECT* pO;
	OBJECT* tO = NULL;
	double topProb = 0.0;
	int i=0;
	while((pO = m_pDet->at(i++)) != NULL)
	{
		IF_CONT(pO->m_topClass != m_iClass);
		IF_CONT(pO->m_topProb < topProb);

		tO = pO;
		topProb = pO->m_topProb;
	}

	vDouble4 bb;
	if(m_bUseTracker)
	{
		if(tO)
		{
			if(m_pTnew->trackState() == track_stop)
			{
				m_pTnew->startTrack(tO->m_bb);
			}
		}

		if(m_pTnew->trackState() == track_update)
		{
			m_iTracker = 1 - m_iTracker;
			m_pTnow = m_pTracker[m_iTracker];
			m_pTnew = m_pTracker[1-m_iTracker];
			m_pTnew->stopTrack();
		}

		if(m_pTnow->trackState() != track_update)
		{
			m_pAM->transit("CC_SEARCH");
			return false;
		}

		bb = *m_pTnow->getBB();
	}
	else
	{
		if(!tO)
		{
			m_pAM->transit("CC_SEARCH");
			return false;
		}

		bb = tO->m_bb;
	}

	m_vTarget.x = bb.midX();
	m_vTarget.y = bb.midY();
	m_pAM->transit("CC_FOLLOW");

	return true;
}

void APcopter_followBase::updateGimbal(void)
{
	m_pAP->m_pMavlink->mountConfigure(m_gimbalConfig);
	m_pAP->m_pMavlink->mountControl(m_gimbalControl);
}

bool APcopter_followBase::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();
	IF_F(pMat->empty());
	IF_F(check()<0);

	string* pState = m_pAM->getCurrentStateName();
	string msg;

	pWin->tabNext();

	if(!isActive())
	{
		msg = "Inactive";
		pWin->addMsg(&msg);
	}
	else if (*pState=="CC_FOLLOW")
	{
		msg = "Following";
		pWin->addMsg(&msg);
	}
	else
	{
		msg = "Searching";
		pWin->addMsg(&msg);
	}

	msg = "Cam Target = (" + f2str(m_vTarget.x) + ", "
							   + f2str(m_vTarget.y) + ", "
					           + f2str(m_vTarget.z) + ", "
				           	   + f2str(m_vTarget.w) + ")";
	pWin->addMsg(&msg);

	pWin->tabPrev();

	return true;
}

bool APcopter_followBase::cli(int& iY)
{
	IF_F(!this->ActionBase::cli(iY));
	IF_F(check()<0);

	string* pState = m_pAM->getCurrentStateName();
	string msg;

	if(!isActive())
	{
		msg = "Inactive";
	}
	else if (*pState=="CC_FOLLOW")
	{
		msg = "Following";
	}
	else
	{
		msg = "Searching";
	}
	COL_MSG;
	iY++;
	mvaddstr(iY, CLI_X_MSG, msg.c_str());

	msg = "Cam Target = (" + f2str(m_vTarget.x) + ", "
				     	 	   + f2str(m_vTarget.y) + ", "
							   + f2str(m_vTarget.z) + ", "
							   + f2str(m_vTarget.w) + ")";
	COL_MSG;
	iY++;
	mvaddstr(iY, CLI_X_MSG, msg.c_str());

	return true;
}

}
