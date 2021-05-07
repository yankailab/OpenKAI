#include "../ArduPilot/_AP_posCtrl.h"

namespace kai
{

_AP_posCtrl::_AP_posCtrl()
{
	m_pAP = NULL;

	m_vP.init();
	m_vTargetP.init();
	m_vTargetP.x = 0.5;
	m_vTargetP.y = 0.5;
	m_vTargetGlobal.init();
	m_vTargetGlobal.z = 10.0;

	m_pRoll = NULL;
	m_pPitch = NULL;
	m_pAlt = NULL;
	m_pYaw = NULL;

	m_sptLocal.vx = 0.0;
	m_sptLocal.vy = 0.0;
	m_sptLocal.vz = 0.0;
	m_sptLocal.x = 0.0;
	m_sptLocal.y = 0.0;
	m_sptLocal.z = 0.0;
	m_sptLocal.yaw = 0.0;
	m_sptLocal.yaw_rate = 0.0;

	m_sptGlobal.vx = 0.0;
	m_sptGlobal.vy = 0.0;
	m_sptGlobal.vz = 0.0;
	m_sptGlobal.lat_int = 0.0;
	m_sptGlobal.lon_int = 0.0;
	m_sptGlobal.alt = 0.0;
	m_sptGlobal.yaw = 0.0;
	m_sptGlobal.yaw_rate = 90.0 * DEG_2_RAD;

}

_AP_posCtrl::~_AP_posCtrl()
{
}

bool _AP_posCtrl::init(void* pKiss)
{
	IF_F(!this->_StateBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("vTargetP", &m_vTargetP);
	pK->v("vTargetGlobal", &m_vTargetGlobal);

	string n;

	n = "";
	pK->v("PIDroll", &n);
	m_pRoll = ( PID*) (pK->getInst(n));

	n = "";
	pK->v("PIDpitch", &n);
	m_pPitch = ( PID*) (pK->getInst(n));

	n = "";
	pK->v("PIDalt", &n);
	m_pAlt = ( PID*) (pK->getInst(n));

	n = "";
	pK->v("PIDyaw", &n);
	m_pYaw = ( PID*) (pK->getInst(n));

	n = "";
	pK->v("_AP_base", &n);
	m_pAP = (_AP_base*) (pK->getInst(n));
	IF_Fl(!m_pAP, n + ": not found");

	return true;
}

bool _AP_posCtrl::start(void)
{
    NULL_F(m_pT);
	return m_pT->start(getUpdate, this);
}

int _AP_posCtrl::check(void)
{
	NULL__(m_pAP, -1);
	NULL__(m_pAP->m_pMav, -1);

	return this->_StateBase::check();
}

void _AP_posCtrl::update(void)
{
	while(m_pT->bRun())
	{
		m_pT->autoFPSfrom();
		this->_StateBase::update();

		setPosLocal();

		m_pT->autoFPSto();
	}
}

void _AP_posCtrl::setPosLocal(void)
{
	IF_(check()<0);

	float dTs = m_pT->getDt() * USEC_2_SEC;
	float p = 0, r = 0, a = 0, y = 0;
	if (m_pRoll)
		r = m_pRoll->update(m_vP.x, m_vTargetP.x, dTs);

	if (m_pPitch)
		p = m_pPitch->update(m_vP.y, m_vTargetP.y, dTs);

	if (m_pAlt)
		a = m_pAlt->update(m_vP.z, m_vTargetP.z, dTs);
	else
		a = m_vP.z;

	if(m_pYaw)
		y = m_pYaw->update(dHdg<float>(m_vTargetP.w, m_vP.w), 0.0, dTs);

	m_sptLocal.coordinate_frame = MAV_FRAME_BODY_OFFSET_NED;
	m_sptLocal.vx = p;		//forward
	m_sptLocal.vy = r;		//right
	m_sptLocal.vz = a;		//down
	m_sptLocal.yaw = 0.0;
	m_sptLocal.yaw_rate = y * DEG_2_RAD;
	m_sptLocal.type_mask = 0b0000010111000111;	//set velocity

	m_pAP->m_pMav->setPositionTargetLocalNED(m_sptLocal);
}

void _AP_posCtrl::setPosGlobal(void)
{
	IF_(check()<0);

	m_sptGlobal.coordinate_frame = MAV_FRAME_GLOBAL_RELATIVE_ALT_INT;
	m_sptGlobal.lat_int = m_vTargetGlobal.x * 1e7;
	m_sptGlobal.lon_int = m_vTargetGlobal.y * 1e7;
	m_sptGlobal.alt = m_vTargetGlobal.z;
	m_sptGlobal.vx = 0.0;
	m_sptGlobal.vy = 0.0;
	m_sptGlobal.vz = 0.0;
	m_sptGlobal.yaw = (float) m_vTargetGlobal.w * DEG_2_RAD;
	m_sptGlobal.type_mask = 0b0000000111111000; //set position
//	if(!m_bYaw)
//		m_sptGlobal.type_mask |= 0b0000110000000000;

	m_pAP->m_pMav->setPositionTargetGlobalINT(m_sptGlobal);
}

void _AP_posCtrl::clear(void)
{
	if (m_pRoll)
		m_pRoll->reset();
	if (m_pPitch)
		m_pPitch->reset();
	if (m_pAlt)
		m_pAlt->reset();
	if (m_pYaw)
		m_pYaw->reset();
}

void _AP_posCtrl::releaseCtrl(void)
{
	IF_(check() < 0);

	m_sptLocal.coordinate_frame = MAV_FRAME_BODY_OFFSET_NED;
	m_sptLocal.x = 0.0;
	m_sptLocal.y = 0.0;
	m_sptLocal.z = 0.0;
	m_sptLocal.vx = 0;
	m_sptLocal.vy = 0;
	m_sptLocal.vz = 0;
	m_sptLocal.yaw_rate = 0.0;
	m_sptLocal.type_mask = 0b0000010111000111;
	m_pAP->m_pMav->setPositionTargetLocalNED(m_sptLocal);
}

void _AP_posCtrl::console(void* pConsole)
{
	NULL_(pConsole);
	this->_StateBase::console(pConsole);

	_Console *pC = (_Console *)pConsole;
	pC->addMsg( "Local NED:");
	pC->addMsg(	"vTargetP = (" + f2str(m_vTargetP.x) + ", "
					+ f2str(m_vTargetP.y) + ", " + f2str(m_vTargetP.z)
					+ ", " + f2str(m_vTargetP.w) + ")",1);
	pC->addMsg(	"vP = (" + f2str(m_vP.x) + ", " + f2str(m_vP.y) + ", "
					+ f2str(m_vP.z) + ", " + f2str(m_vP.w) + ")",1);
	pC->addMsg(	"setV = (" + f2str(m_sptLocal.vx,7) + ", " + f2str(m_sptLocal.vy,7)
					+ ", " + f2str(m_sptLocal.vz,7) + ")",1);
	pC->addMsg(	"setP = (" + f2str(m_sptLocal.x,7) + ", " + f2str(m_sptLocal.y,7)
					+ ", " + f2str(m_sptLocal.z,7) + ")",1);
	pC->addMsg(	"yawRate=" + f2str(m_sptLocal.yaw_rate) + ", yaw=" + f2str(m_sptLocal.yaw),1);

	pC->addMsg( "Global INT:");
	pC->addMsg(	"vTargetGlobal = (" + lf2str(m_vTargetGlobal.x,7) + ", "
					+ lf2str(m_vTargetGlobal.y,7) + ", " + lf2str(m_vTargetGlobal.z,2)
					+ ", " + lf2str(m_vTargetGlobal.w,2) + ")",1);

}

}
