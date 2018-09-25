#include "APcopter_followGlobal.h"

namespace kai
{

APcopter_followGlobal::APcopter_followGlobal()
{
	m_utmV.init();
	m_vCamNEA.init();
	m_vCamRelNEA.init();
	m_vTargetNEA.init();
	m_vMyDestNEA.init();
	m_vMove.init();
}

APcopter_followGlobal::~APcopter_followGlobal()
{
}

bool APcopter_followGlobal::init(void* pKiss)
{
	IF_F(!this->APcopter_followBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("dN", &m_vCamRelNEA.x);
	pK->v("dE", &m_vCamRelNEA.y);
	pK->v("dA", &m_vCamRelNEA.z);
//	pK->v("rH", &m_vRelPos.w);

	return true;
}

int APcopter_followGlobal::check(void)
{
	return this->APcopter_followBase::check();
}

void APcopter_followGlobal::update(void)
{
	this->APcopter_followBase::update();
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
	IF_(!find());

	//global vehicle pos
	m_GPS.update(m_pAP->m_pMavlink);
	m_utmV = m_GPS.getUTM();

	//desired target position in local NEA
	vDouble3 vCamNEA = m_vCamRelNEA;
	vCamNEA.x += m_utmV.m_altRel * tan(m_vGimbalRad.x);	//Northing

	//target position in local NEA
	_VisionBase* pV = m_pDet->m_pVision;
	vDouble2 cAngle;
	pV->info(NULL, NULL, &cAngle);

	double radN = (m_vTarget.y - 0.5) * cAngle.y * DEG_RAD + m_vGimbalRad.x;
	double radE = (m_vTarget.x - 0.5) * cAngle.x * DEG_RAD + m_vGimbalRad.y;

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
}

bool APcopter_followGlobal::draw(void)
{
	IF_F(!this->APcopter_followBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();
	IF_F(pMat->empty());
	IF_F(check()<0);

	string msg;

	pWin->tabNext();

	msg = "UTM Vehicle: N=" + f2str(m_utmV.m_northing)
					+ ", E=" + f2str(m_utmV.m_easting)
					+ ", A_abs=" + f2str(m_utmV.m_altAbs)
					+ ", A_rel=" + f2str(m_utmV.m_altRel)
					+ ", Hdg=" + f2str(m_utmV.m_hdg);
	pWin->addMsg(&msg);

	msg = "Cam: N=" + f2str(m_vCamNEA.x)
					+ ", E=" + f2str(m_vCamNEA.y)
					+ ", A=" + f2str(m_vCamNEA.z);
	pWin->addMsg(&msg);

	msg = "Cam Rel: N=" + f2str(m_vCamRelNEA.x)
					+ ", E=" + f2str(m_vCamRelNEA.y)
					+ ", A=" + f2str(m_vCamRelNEA.z);
	pWin->addMsg(&msg);

	msg = "Target: N=" + f2str(m_vTargetNEA.x)
					+ ", E=" + f2str(m_vTargetNEA.y)
					+ ", A=" + f2str(m_vTargetNEA.z);
	pWin->addMsg(&msg);

	msg = "My Dest: N=" + f2str(m_vMyDestNEA.x)
					+ ", E=" + f2str(m_vMyDestNEA.y)
					+ ", A=" + f2str(m_vMyDestNEA.z);
	pWin->addMsg(&msg);

	msg = "vMove: N=" + f2str(m_vMove.x)
				    + ", E=" + f2str(m_vMove.y)
					+ ", A=" + f2str(m_vMove.z);
	pWin->addMsg(&msg);

	pWin->tabPrev();

	return true;
}

bool APcopter_followGlobal::cli(int& iY)
{
	IF_F(!this->APcopter_followBase::cli(iY));
	IF_F(check()<0);

	string msg;

	msg = "UTM Vehicle: N=" + f2str(m_utmV.m_northing)
					+ ", E=" + f2str(m_utmV.m_easting)
					+ ", A_abs=" + f2str(m_utmV.m_altAbs)
					+ ", A_rel=" + f2str(m_utmV.m_altRel)
					+ ", Hdg=" + f2str(m_utmV.m_hdg);
	COL_MSG;
	iY++;
	mvaddstr(iY, CLI_X_MSG, msg.c_str());

	msg = "Cam: N=" + f2str(m_vCamNEA.x)
					+ ", E=" + f2str(m_vCamNEA.y)
					+ ", A=" + f2str(m_vCamNEA.z);
	COL_MSG;
	iY++;
	mvaddstr(iY, CLI_X_MSG, msg.c_str());

	msg = "Cam Rel: N=" + f2str(m_vCamRelNEA.x)
					+ ", E=" + f2str(m_vCamRelNEA.y)
					+ ", A=" + f2str(m_vCamRelNEA.z);
	COL_MSG;
	iY++;
	mvaddstr(iY, CLI_X_MSG, msg.c_str());

	msg = "Target: N=" + f2str(m_vTargetNEA.x)
					+ ", E=" + f2str(m_vTargetNEA.y)
					+ ", A=" + f2str(m_vTargetNEA.z);
	COL_MSG;
	iY++;
	mvaddstr(iY, CLI_X_MSG, msg.c_str());

	msg = "My Dest: N=" + f2str(m_vMyDestNEA.x)
					+ ", E=" + f2str(m_vMyDestNEA.y)
					+ ", A=" + f2str(m_vMyDestNEA.z);
	COL_MSG;
	iY++;
	mvaddstr(iY, CLI_X_MSG, msg.c_str());

	msg = "vMove: N=" + f2str(m_vMove.x)
				    + ", E=" + f2str(m_vMove.y)
					+ ", A=" + f2str(m_vMove.z);
	COL_MSG;
	iY++;
	mvaddstr(iY, CLI_X_MSG, msg.c_str());

	return true;
}

}
