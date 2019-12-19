#include "_APcopter_dynamicLanding.h"

namespace kai
{

_APcopter_dynamicLanding::_APcopter_dynamicLanding()
{
	m_pAP = NULL;
	m_pIRlock = NULL;
	m_pAruco = NULL;

	m_vSpeed.init(0.3);
	m_degLand = 5.0;
}

_APcopter_dynamicLanding::~_APcopter_dynamicLanding()
{
}

bool _APcopter_dynamicLanding::init(void* pKiss)
{
	IF_F(!this->_AutopilotBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("vSpeed",&m_vSpeed);
	pK->v("degLand",&m_degLand);

	string iName;
	iName = "";
	pK->v("APcopter_base", &iName);
	m_pAP = (_APcopter_base*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pAP, iName + ": not found");

	iName = "";
	pK->v("_IRLock", &iName);
	m_pIRlock = (_DetectorBase*) (pK->root()->getChildInst(iName));

	iName = "";
	pK->v("_Aruco", &iName);
	m_pAruco = (_DetectorBase*) (pK->root()->getChildInst(iName));

	return true;
}

bool _APcopter_dynamicLanding::start(void)
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

int _APcopter_dynamicLanding::check(void)
{
	NULL__(m_pAP,-1);

	return this->_AutopilotBase::check();
}

void _APcopter_dynamicLanding::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		this->_AutopilotBase::update();

		updateTarget();

		this->autoFPSto();
	}
}

void _APcopter_dynamicLanding::updateTarget(void)
{
	this->_AutopilotBase::update();
	IF_(check()<0);
	if(m_pAP->getApMode() != GUIDED)
	{
//		releaseCtrl();
//		m_tO.init();
//		return;
	}

	OBJECT* pO = NULL;
	if(m_pIRlock)
		pO = m_pIRlock->at(0);
	else if(m_pAruco)
		pO = m_pAruco->at(0);

	if(!pO)
	{
		releaseCtrl();
		m_tO.init();
		return;
	}

	m_tO = *pO;
	vFloat2 vAngle = (m_tO.m_bb.center());
	float x = (vAngle.x > 0.0)?m_vSpeed.x:-m_vSpeed.x;
	float y = (vAngle.y > 0.)?m_vSpeed.y:-m_vSpeed.y;
	float a = (vAngle.x + vAngle.y < m_degLand*DEG_RAD)?m_vSpeed.z:0.0;

	m_spt.coordinate_frame = MAV_FRAME_BODY_OFFSET_NED;
	m_spt.type_mask = 0b0000000111000111;

	m_spt.vx = y;		//forward
	m_spt.vy = x;		//right
	m_spt.vz = a;		//down
	m_spt.type_mask &= 0b1111111111000111;
	m_spt.yaw_rate = (float) 0.0 * DEG_RAD;
	m_spt.yaw = (float) 0.0 * DEG_RAD;

	m_pAP->m_pMavlink->setPositionTargetLocalNED(m_spt);
}

void _APcopter_dynamicLanding::releaseCtrl(void)
{
	IF_(check() < 0);

	m_spt.coordinate_frame = MAV_FRAME_BODY_OFFSET_NED;
	m_spt.x = 0.0;
	m_spt.y = 0.0;
	m_spt.z = 0.0;
	m_spt.vx = 0;
	m_spt.vy = 0;
	m_spt.vz = 0;
	m_spt.yaw = 0.0;
	m_spt.yaw_rate = 0.0;
	m_spt.type_mask = 0b0000000111000111;
	m_pAP->m_pMavlink->setPositionTargetLocalNED(m_spt);
}

void _APcopter_dynamicLanding::draw(void)
{
	this->_AutopilotBase::draw();
	IF_(check()<0);

	addMsg(
			"Set target: V = (" + f2str(m_spt.vx) + ", " + f2str(m_spt.vy)
					+ ", " + f2str(m_spt.vz) + "), P = (" + f2str(m_spt.x)
					+ ", " + f2str(m_spt.y) + ", " + f2str(m_spt.z) + ")",1);

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
