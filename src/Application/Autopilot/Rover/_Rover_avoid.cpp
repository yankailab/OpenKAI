#include "_Rover_avoid.h"

namespace kai
{

_Rover_avoid::_Rover_avoid()
{
	m_pDV = NULL;
	m_d = -1.0;
	m_minD = 3.0;
	m_vRoi.init();
	m_vRoi.z = 1.0;
	m_vRoi.w = 1.0;

	m_ctrl.init();
	m_pCtrl = (void*)&m_ctrl;
}

_Rover_avoid::~_Rover_avoid()
{
}

bool _Rover_avoid::init(void* pKiss)
{
	IF_F(!this->_AutopilotBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

	pK->v("minD", &m_minD);
	pK->v("vRoi", &m_vRoi);

	string iName;
	iName = "";
	F_ERROR_F(pK->v("_DepthVisionBase", &iName));
	m_pDV = (_DepthVisionBase*) (pK->root()->getChildInst(iName));
	NULL_Fl(m_pDV, iName+": not found");

	return true;
}

bool _Rover_avoid::start(void)
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

int _Rover_avoid::check(void)
{
	NULL__(m_pAB, -1);
	NULL__(m_pAB->m_pCtrl, -1);
	NULL__(m_pDV, -1);

	return this->_AutopilotBase::check();
}

void _Rover_avoid::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		this->_AutopilotBase::update();
		updateAvoid();

		this->autoFPSto();
	}
}

void _Rover_avoid::updateAvoid(void)
{
	IF_(check()<0);

	ROVER_CTRL* pCtrl = (ROVER_CTRL*)m_pAB->m_pCtrl;
	m_ctrl.m_hdg = pCtrl->m_hdg;
	m_ctrl.m_nSpeed = pCtrl->m_nSpeed;
	m_ctrl.m_targetHdgOffset = pCtrl->m_targetHdgOffset;

	m_d = m_pDV->d(&m_vRoi);

	if(m_d > m_minD || m_d < 0.0)
	{
		m_ctrl.m_nTargetSpeed = pCtrl->m_nTargetSpeed;
		return;
	}

	m_ctrl.m_nTargetSpeed = 0.0;

}

void _Rover_avoid::draw(void)
{
	this->_AutopilotBase::draw();
	IF_(check()<0);

	string msg = "d=" + f2str(m_d)
			+ ", minD=" + f2str(m_minD)
			+ ", nTargetSpeed=" + f2str(m_ctrl.m_nTargetSpeed);
	addMsg(msg);

	IF_(!checkWindow());
	Mat* pMat = ((Window*) this->m_pWindow)->getFrame()->m();

	vInt2 cs;
	cs.x = pMat->cols;
	cs.y = pMat->rows;
	Rect r = convertBB<vInt4>(convertBB(m_vRoi, cs));
	rectangle(*pMat, r, Scalar(255,255,0), 2);

}

}
