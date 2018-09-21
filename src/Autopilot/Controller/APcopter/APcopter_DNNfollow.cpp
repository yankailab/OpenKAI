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
	m_vTargetPos.init();
	m_vRelPos.init();
	m_vMyPos.init();
	m_bTarget = false;
	m_bUseTracker = false;
	m_bUseFOL = true;

	m_vGimbal.init();
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

	pK->v("dN", &m_vRelPos.x);
	pK->v("dE", &m_vRelPos.y);
	pK->v("dA", &m_vRelPos.z);
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

	if(!m_bUseFOL)
	{
		iName = "";
		pK->v("APcopter_posCtrlRC", &iName);
		m_pPC = (APcopter_posCtrlRC*) (pK->parent()->getChildInst(iName));
		IF_Fl(!m_pPC, iName + ": not found");
	}

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
	if(!m_bUseFOL)
		NULL__(m_pPC,-1);

	return this->ActionBase::check();
}

void APcopter_DNNfollow::update(void)
{
	this->ActionBase::update();
	IF_(check()<0);
	if(!isActive())
	{
		m_bTarget = false;
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

			m_bTarget = false;
			m_pAM->transit("CC_SEARCH");
			return;
		}

		bb = *m_pTracker->getBB();
	}
	else
	{
		if(!pO)
		{
			m_bTarget = false;
			m_pAM->transit("CC_SEARCH");
			return;
		}

		bb = pO->m_bb;
	}

	//calc positions
	m_GPS.update(m_pAP->m_pMavlink);
	UTM_POS utmVehicle = m_GPS.getUTM();
	vDouble3 vGimbalRad = m_vGimbal * DEG_RAD;

	//desired target position in local NEA
	vDouble3 vCamNEA = m_vRelPos;
	vCamNEA.x += utmVehicle.m_alt * tan(vGimbalRad.x);	//Northing

	//target position in local NEA
	_VisionBase* pV = m_pDet->m_pVision;
	vDouble2 cAngle;
	pV->info(NULL, NULL, &cAngle);

	double radN = (bb.midY() - 0.5) * cAngle.y * DEG_RAD + vGimbalRad.x;
	double radE = (bb.midX() - 0.5) * cAngle.x * DEG_RAD + vGimbalRad.y;

	vDouble3 vTargetNEA;
	vTargetNEA.x = utmVehicle.m_alt * tan(radN);				//N
	vTargetNEA.y = utmVehicle.m_alt / cos(radN) * tan(radE);	//E
	vTargetNEA.z = 0.0;

	//TODO


	m_bTarget = true;
	m_pAM->transit("CC_FOLLOW");

	if(m_bUseFOL)
	{
		//Use FOLLOW mode

	}
	else
	{
		//Use RC override
		m_pPC->setCtrl(RC_CHAN_ROLL,true);
		m_pPC->setCtrl(RC_CHAN_PITCH,true);

//		m_pPC->setPos(m_vPos);
//		m_pPC->setTargetPos(m_vTarget);
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

	string msg = *this->getName() + ": ";

	if(!isActive())
	{
		msg += "Inactive";
	}
	else if (m_bTarget)
	{
//		circle(*pMat, Point(m_vPos.x * pMat->cols,
//							m_vPos.y * pMat->rows),
//				pMat->cols * pMat->rows * 0.00005, Scalar(0, 0, 255), 2);
//
//		msg += "Pos = (" + f2str(m_vPos.x) + ", "
//							   + f2str(m_vPos.y) + ", "
//							   + f2str(m_vPos.z) + ", "
//							   + f2str(m_vPos.w) + ")";
	}
	else
	{
		msg += "Not found";
	}

	pWin->addMsg(&msg);

//	msg = "Target pos = (" + f2str(m_vTarget.x) + ", "
//						   + f2str(m_vTarget.y) + ", "
//						   + f2str(m_vTarget.z) + ", "
//						   + f2str(m_vTarget.w) + ")";
//
//	pWin->addMsg(&msg);
//
//	circle(*pMat, Point(m_vTarget.x * pMat->cols,
//						m_vTarget.y * pMat->rows),
//			pMat->cols * pMat->rows * 0.00005, Scalar(0, 255, 0), 2);

	return true;
}

bool APcopter_DNNfollow::cli(int& iY)
{
	IF_F(!this->ActionBase::cli(iY));
	IF_F(check()<0);

	string msg;
	if(!isActive())
	{
		msg = "Inactive";
	}
	else if (m_bTarget)
	{
//		msg = "Pos = (" + f2str(m_vPos.x) + ", "
//							   + f2str(m_vPos.y) + ", "
//							   + f2str(m_vPos.z) + ", "
//							   + f2str(m_vPos.w) + ")";
	}
	else
	{
		msg = "Not found";
	}

	COL_MSG;
	iY++;
	mvaddstr(iY, CLI_X_MSG, msg.c_str());

//	msg = "Target pos = (" + f2str(m_vTarget.x) + ", "
//						   + f2str(m_vTarget.y) + ", "
//						   + f2str(m_vTarget.z) + ", "
//						   + f2str(m_vTarget.w) + ")";

	COL_MSG;
	iY++;
	mvaddstr(iY, CLI_X_MSG, msg.c_str());

	return true;
}

}
