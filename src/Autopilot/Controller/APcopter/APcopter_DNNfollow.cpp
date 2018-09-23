#include "APcopter_DNNfollow.h"

namespace kai
{

APcopter_DNNfollow::APcopter_DNNfollow()
{
	m_pAP = NULL;
	m_pPC = NULL;
	m_pDet = NULL;
	m_pTracker = NULL;
	m_tStampDet = 0;

	m_iClass = -1;
	m_bUseTracker = false;

	m_utmV.init();
	m_vCamNEA.init();
	m_vCamRelNEA.init();
	m_vTargetNEA.init();
	m_vMyDestNEA.init();
	m_vGimbal.init();

	m_vTarget.init();
}

APcopter_DNNfollow::~APcopter_DNNfollow()
{
}

bool APcopter_DNNfollow::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK,iClass);
	KISSm(pK,bUseTracker);

	pK->v("dN", &m_vCamRelNEA.x);
	pK->v("dE", &m_vCamRelNEA.y);
	pK->v("dA", &m_vCamRelNEA.z);
//	pK->v("rH", &m_vRelPos.w);

	Kiss* pG = pK->o("gimbal");
	if(!pG->empty())
	{
		pG->v("pitch", &m_vGimbal.x);
		pG->v("roll", &m_vGimbal.y);
		pG->v("yaw", &m_vGimbal.z);
	}

	//link
	string iName;

	if(m_bUseTracker)
	{
		iName = "";
		pK->v("_TrackerBase", &iName);
		m_pTracker = (_TrackerBase*) (pK->root()->getChildInst(iName));
		IF_Fl(!m_pTracker, iName + ": not found");
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
	pK->v("APcopter_posCtrlRC", &iName);
	m_pPC = (APcopter_posCtrlRC*) (pK->parent()->getChildInst(iName));

	return true;
}

int APcopter_DNNfollow::check(void)
{
	NULL__(m_pAP,-1);
	NULL__(m_pAP->m_pMavlink,-1);
	NULL__(m_pDet,-1);
	_VisionBase* pV = m_pDet->m_pVision;
	NULL__(pV,-1);
	if(m_bUseTracker)
		NULL__(m_pTracker,-1);

	return this->ActionBase::check();
}

void APcopter_DNNfollow::update(void)
{
	this->ActionBase::update();
	IF_(check()<0);
	if(!isActive())
	{
		m_pDet->sleep();
		if(m_bUseTracker)
			m_pTracker->stopTrack();

		return;
	}

	if(m_bStateChanged)
	{
		m_pDet->wakeUp();
	}

	updateGimbal();

	//find target
	OBJECT* pO = newFound();
	vDouble4 bb;

	if(m_bUseTracker)
	{
		if(!m_pTracker->bTracking())
		{
			if(pO)
			{
				m_pTracker->startTrack(pO->m_bb);
			}

			m_pAM->transit("CC_SEARCH");
			return;
		}

		bb = *m_pTracker->getBB();
	}
	else
	{
		if(!pO)
		{
			m_pAM->transit("CC_SEARCH");
			return;
		}

		bb = pO->m_bb;
	}

	m_vTarget.x = bb.midX();
	m_vTarget.y = bb.midY();

	if(m_pAP->apMode()==FOLLOW || !m_pPC)
	{
		m_pAM->transit("CC_FOLLOW_FOL");

		//global vehicle pos
		m_GPS.update(m_pAP->m_pMavlink);
		m_utmV = m_GPS.getUTM();
		vDouble3 vGimbalRad = m_vGimbal * DEG_RAD;

		//desired target position in local NEA
		vDouble3 vCamNEA = m_vCamRelNEA;
		vCamNEA.x += m_utmV.m_alt * tan(vGimbalRad.x);	//Northing

		//target position in local NEA
		_VisionBase* pV = m_pDet->m_pVision;
		vDouble2 cAngle;
		pV->info(NULL, NULL, &cAngle);

		double radN = (m_vTarget.y - 0.5) * cAngle.y * DEG_RAD + vGimbalRad.x;
		double radE = (m_vTarget.x - 0.5) * cAngle.x * DEG_RAD + vGimbalRad.y;

		vDouble3 vTargetNEA;
		vTargetNEA.x = m_utmV.m_alt * tan(radN);				//N
		vTargetNEA.y = m_utmV.m_alt / cos(radN) * tan(radE);	//E
		vTargetNEA.z = 0.0;

		//global pos that vehicle should go
		vDouble3 dPos = vTargetNEA - vCamNEA;
		dPos.z = 0.0;
		UTM_POS utmDest = m_GPS.getPos(dPos);
		LL_POS vLL = m_GPS.UTM2LL(utmDest);

		mavlink_global_position_int_t D;
		D.lat = vLL.m_lat * 1e7;
		D.lon = vLL.m_lng * 1e7;
		D.relative_alt = vLL.m_alt * 1000;
		D.alt = m_pAP->m_pMavlink->m_msg.global_position_int.alt;
		D.hdg = m_pAP->m_pMavlink->m_msg.global_position_int.hdg;
		D.vx = 0;
		D.vy = 0;
		D.vz = 0;
		m_pAP->m_pMavlink->globalPositionInt(D);
	}
	else
	{
		m_pAM->transit("CC_FOLLOW_RC");

		m_vTarget.x = m_vTarget.x - 0.5;	//roll
		m_vTarget.y = m_vTarget.y - 0.5;	//pitch
		m_vTarget.z = 0.0;
		m_vTarget.w = 0.0;
		m_pPC->setPos(m_vTarget);

		vDouble4 vCam;
		vCam.x = 0.5;
		vCam.y = 0.5;
		vCam.z = 0.0;
		vCam.w = 0.0;
		m_pPC->setTargetPos(vCam);

		//Use RC override
		m_pPC->setCtrl(RC_CHAN_ROLL,true);
		m_pPC->setCtrl(RC_CHAN_PITCH,true);
	}
}

OBJECT* APcopter_DNNfollow::newFound(void)
{
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

	return tO;
}

void APcopter_DNNfollow::updateGimbal(void)
{
	//enable camera gimbal and set to the right angle
	mavlink_mount_configure_t D;
	D.stab_pitch = 1;
	D.stab_roll = 1;
	D.stab_yaw = 1;
	D.mount_mode = 2;
	m_pAP->m_pMavlink->mountConfigure(D);

	mavlink_mount_control_t C;
	C.input_a = m_vGimbal.x * 100;	//pitch
	C.input_b = m_vGimbal.y * 100;	//roll
	C.input_c = m_vGimbal.z * 100;	//yaw
	C.save_position = 0;
	m_pAP->m_pMavlink->mountControl(C);
}

bool APcopter_DNNfollow::draw(void)
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
	else if (*pState=="CC_FOLLOW_RC")
	{
		msg = "RC mode";
		pWin->addMsg(&msg);

		circle(*pMat,
				Point(m_vTarget.x * pMat->cols,
				  	  m_vTarget.y * pMat->rows),
				pMat->cols * pMat->rows * 0.00005, Scalar(0, 0, 255), 2);
	}
	else if (*pState=="CC_FOLLOW_FOL")
	{
		msg = "FOL mode";
		pWin->addMsg(&msg);
	}
	else
	{
		msg = "Searching";
		pWin->addMsg(&msg);
	}

	msg = "UTM Vehicle: N=" + f2str(m_utmV.m_northing)
					+ ", E=" + f2str(m_utmV.m_easting)
					+ ", A=" + f2str(m_utmV.m_alt)
					+ ", Hdg=" + f2str(m_utmV.m_hdg);
	pWin->addMsg(&msg);

	msg = "Cam NEA: N=" + f2str(m_vCamNEA.x)
					+ ", E=" + f2str(m_vCamNEA.y)
					+ ", A=" + f2str(m_vCamNEA.z);
	pWin->addMsg(&msg);

	msg = "Cam Rel NEA: N=" + f2str(m_vCamRelNEA.x)
					+ ", E=" + f2str(m_vCamRelNEA.y)
					+ ", A=" + f2str(m_vCamRelNEA.z);
	pWin->addMsg(&msg);

	msg = "Target NEA: N=" + f2str(m_vTargetNEA.x)
					+ ", E=" + f2str(m_vTargetNEA.y)
					+ ", A=" + f2str(m_vTargetNEA.z);
	pWin->addMsg(&msg);

	msg = "My Dest NEA: N=" + f2str(m_vMyDestNEA.x)
					+ ", E=" + f2str(m_vMyDestNEA.y)
					+ ", A=" + f2str(m_vMyDestNEA.z);
	pWin->addMsg(&msg);

	msg = "Cam Target Pos = (" + f2str(m_vTarget.x) + ", "
				     + f2str(m_vTarget.y) + ", "
					 + f2str(m_vTarget.z) + ", "
					 + f2str(m_vTarget.w) + ")";
	pWin->addMsg(&msg);

	pWin->tabPrev();

	return true;
}

bool APcopter_DNNfollow::cli(int& iY)
{
	IF_F(!this->ActionBase::cli(iY));
	IF_F(check()<0);

	string* pState = m_pAM->getCurrentStateName();
	string msg;

	if(!isActive())
	{
		msg = "Inactive";
	}
	else if (*pState=="CC_FOLLOW_RC")
	{
		msg = "RC mode";
	}
	else if (*pState=="CC_FOLLOW_FOL")
	{
		msg = "FOL mode";
	}
	else
	{
		msg = "Searching";
	}
	COL_MSG;
	iY++;
	mvaddstr(iY, CLI_X_MSG, msg.c_str());


	msg = "UTM Vehicle: N=" + f2str(m_utmV.m_northing)
					+ ", E=" + f2str(m_utmV.m_easting)
					+ ", A=" + f2str(m_utmV.m_alt)
					+ ", Hdg=" + f2str(m_utmV.m_hdg);
	COL_MSG;
	iY++;
	mvaddstr(iY, CLI_X_MSG, msg.c_str());

	msg = "Cam NEA: N=" + f2str(m_vCamNEA.x)
					+ ", E=" + f2str(m_vCamNEA.y)
					+ ", A=" + f2str(m_vCamNEA.z);
	COL_MSG;
	iY++;
	mvaddstr(iY, CLI_X_MSG, msg.c_str());

	msg = "Cam Rel NEA: N=" + f2str(m_vCamRelNEA.x)
					+ ", E=" + f2str(m_vCamRelNEA.y)
					+ ", A=" + f2str(m_vCamRelNEA.z);
	COL_MSG;
	iY++;
	mvaddstr(iY, CLI_X_MSG, msg.c_str());

	msg = "Target NEA: N=" + f2str(m_vTargetNEA.x)
					+ ", E=" + f2str(m_vTargetNEA.y)
					+ ", A=" + f2str(m_vTargetNEA.z);
	COL_MSG;
	iY++;
	mvaddstr(iY, CLI_X_MSG, msg.c_str());

	msg = "My Dest NEA: N=" + f2str(m_vMyDestNEA.x)
					+ ", E=" + f2str(m_vMyDestNEA.y)
					+ ", A=" + f2str(m_vMyDestNEA.z);
	COL_MSG;
	iY++;
	mvaddstr(iY, CLI_X_MSG, msg.c_str());

	msg = "Cam Target Pos = (" + f2str(m_vTarget.x) + ", "
				     + f2str(m_vTarget.y) + ", "
					 + f2str(m_vTarget.z) + ", "
					 + f2str(m_vTarget.w) + ")";
	COL_MSG;
	iY++;
	mvaddstr(iY, CLI_X_MSG, msg.c_str());

	return true;
}

}
