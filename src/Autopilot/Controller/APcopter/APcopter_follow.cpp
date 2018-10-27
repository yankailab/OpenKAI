#include "APcopter_follow.h"

namespace kai
{

APcopter_follow::APcopter_follow()
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
	m_mountControl.input_a = m_vGimbal.x * 100;	//pitch
	m_mountControl.input_b = m_vGimbal.y * 100;	//roll
	m_mountControl.input_c = m_vGimbal.z * 100;	//yaw
	m_mountControl.save_position = 0;

	m_mountConfig.stab_pitch = 0;
	m_mountConfig.stab_roll = 0;
	m_mountConfig.stab_yaw = 0;
	m_mountConfig.mount_mode = 2;

	m_pPC = NULL;
	m_vCam.init();
	m_vCam.x = 0.5;
	m_vCam.y = 0.5;

}

APcopter_follow::~APcopter_follow()
{
}

bool APcopter_follow::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK,iClass);
	KISSm(pK,bUseTracker);
	pK->v("camX", &m_vCam.x);
	pK->v("camY", &m_vCam.y);
	pK->v("camYaw", &m_vCam.w);

	Kiss* pG = pK->o("gimbal");
	if(!pG->empty())
	{
		pG->v("pitch", &m_vGimbal.x);
		pG->v("roll", &m_vGimbal.y);
		pG->v("yaw", &m_vGimbal.z);

		m_mountControl.input_a = m_vGimbal.x * 100;	//pitch
		m_mountControl.input_b = m_vGimbal.y * 100;	//roll
		m_mountControl.input_c = m_vGimbal.z * 100;	//yaw
		m_mountControl.save_position = 0;

		pG->v("stabPitch", &m_mountConfig.stab_pitch);
		pG->v("stabRoll", &m_mountConfig.stab_roll);
		pG->v("stabYaw", &m_mountConfig.stab_yaw);
		pG->v("mountMode", &m_mountConfig.mount_mode);
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

	iName = "";
	pK->v("APcopter_posCtrlBase", &iName);
	m_pPC = (APcopter_posCtrlBase*) (pK->parent()->getChildInst(iName));
	IF_Fl(!m_pPC, iName + ": not found");

	return true;
}

int APcopter_follow::check(void)
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

void APcopter_follow::update(void)
{
	this->ActionBase::update();
	IF_(check()<0);
	if(!isActive())
	{
		m_pDet->goSleep();
		if(m_bUseTracker)
		{
			m_pTracker[0]->stopTrack();
			m_pTracker[1]->stopTrack();
		}

		return;
	}

	if(m_bStateChanged)
	{
		m_pDet->wakeUp();
	}

	updateGimbal();
	IF_(!find());

	m_pPC->setPos(m_vTarget);
	m_pPC->setTargetPos(m_vCam);


	/*
	//global vehicle pos
	m_GPS.update(m_pAP->m_pMavlink);
	m_utmV = m_GPS.getUTM();

	//desired target position in local NEA
	vDouble3 vCamNEA = m_vCamRelNEA;
	vCamNEA.x += m_utmV.m_altRel * tan(m_vGimbal.x * DEG_RAD);	//Northing

	//target position in local NEA
	_VisionBase* pV = m_pDet->m_pVision;
	vDouble2 cAngle;
	pV->info(NULL, NULL, &cAngle);

	double radN = (m_vTarget.y - 0.5) * cAngle.y * DEG_RAD + m_vGimbal.x * DEG_RAD;
	double radE = (m_vTarget.x - 0.5) * cAngle.x * DEG_RAD + m_vGimbal.y * DEG_RAD;

	vDouble3 vTargetNEA;
	vTargetNEA.x = m_utmV.m_altRel * tan(radN);				//N
	vTargetNEA.y = m_utmV.m_altRel / cos(radN) * tan(radE);	//E
	vTargetNEA.z = 0.0;

	//global pos that vehicle should go
	m_vMove = vTargetNEA - vCamNEA;
	m_vMove.z = 0.0;
	UTM_POS utmDest = m_GPS.getPos(m_vMove);
	LL_POS vLL = m_GPS.UTM2LL(utmDest);

	uint32_t apMode = m_pAP->apMode();

	if(apMode == FOLLOW)
	{
		mavlink_global_position_int_t D;
		D.lat = vLL.m_lat * 1e7;
		D.lon = vLL.m_lng * 1e7;
		D.relative_alt = vLL.m_altRel * 1000;
		D.alt = m_pAP->m_pMavlink->m_msg.global_position_int.alt;
		D.hdg = m_pAP->m_pMavlink->m_msg.global_position_int.hdg;
		D.vx = 0;
		D.vy = 0;
		D.vz = 0;
		m_pAP->m_pMavlink->globalPositionInt(D);
	}
	else if(apMode == GUIDED)
	{
		mavlink_set_position_target_global_int_t D;
		D.coordinate_frame = MAV_FRAME_GLOBAL_RELATIVE_ALT_INT;
		D.lat_int = vLL.m_lat * 1e7;
		D.lon_int = vLL.m_lng * 1e7;
		D.alt = vLL.m_altRel;
		D.vx = 0;
		D.vy = 0;
		D.vz = 0;
		D.coordinate_frame = MAV_FRAME_BODY_OFFSET_NED;
		D.type_mask = 0b0000111111111000;
		m_pAP->m_pMavlink->setPositionTargetGlobalINT(D);
	}
	 *
	 */
}

bool APcopter_follow::find(void)
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

void APcopter_follow::updateGimbal(void)
{
	m_pAP->m_pMavlink->mountConfigure(m_mountConfig);
	m_pAP->m_pMavlink->mountControl(m_mountControl);

	mavlink_param_set_t D;
	D.param_type = MAV_PARAM_TYPE_INT8;
	string id;

	D.param_value = m_mountConfig.stab_pitch;
	id = "MNT_STAB_TILT";
	strcpy(D.param_id, id.c_str());
	m_pAP->m_pMavlink->param_set(D);

	D.param_value = m_mountConfig.stab_roll;
	id = "MNT_STAB_ROLL";
	strcpy(D.param_id,id.c_str());
	m_pAP->m_pMavlink->param_set(D);
}

bool APcopter_follow::draw(void)
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

bool APcopter_follow::cli(int& iY)
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
