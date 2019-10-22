#include "_Rover_avoid.h"

namespace kai
{

_Rover_avoid::_Rover_avoid()
{
	m_pDet = NULL;
	m_pPID = NULL;
	m_ctrl.init();

	m_obs.init();
	m_dStop = 1.0;
}

_Rover_avoid::~_Rover_avoid()
{
}

bool _Rover_avoid::init(void* pKiss)
{
	IF_F(!this->_AutopilotBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

	pK->v("dStop", &m_dStop);

	string iName;
	iName = "";
	F_ERROR_F(pK->v("_DetectorBase", &iName));
	m_pDet = (_DetectorBase*) (pK->root()->getChildInst(iName));
	NULL_Fl(m_pDet, iName+": not found");

	iName = "";
	pK->v("PID", &iName);
	m_pPID = (PIDctrl*) (pK->root()->getChildInst(iName));
	NULL_Fl(m_pPID, iName+": not found");

	return true;
}

int _Rover_avoid::check(void)
{
	NULL__(m_pDet, -1);
	NULL__(m_pPID, -1);

	return 0;
}

void _Rover_avoid::update(void)
{
	this->_AutopilotBase::update();
	IF_(check()<0);
	IF_(!bActive());

	ROVER_CTRL* pCtrl = (ROVER_CTRL*)m_pAB->m_pCtrl;
	m_ctrl.m_hdg = pCtrl->m_hdg;
	m_ctrl.m_nSpeed = pCtrl->m_nTargetSpeed;
	m_ctrl.m_targetHdg = pCtrl->m_targetHdg;

	OBJECT o;
	o.init();
	OBJECT* pO;
	int i=0;
	while((pO = m_pDet->at(i++)) != NULL)
	{
		if(o.m_topClass>=0)
		{
			IF_CONT(pO->m_dist > o.m_dist);
		}
		o = *pO;
		o.m_topClass = 0;
	}

	if(o.m_topClass<0)
	{
		m_obs.init();
		m_ctrl.m_nTargetSpeed = pCtrl->m_nTargetSpeed;
		return;
	}

	m_obs = o;
	float dBrake = o.m_dist - m_dStop;
	if(dBrake < 0)dBrake = 0.0;

	m_ctrl.m_nTargetSpeed = constrain(
			m_pPID->update(dBrake, 0.0, m_tStamp),
			0.0f,
			pCtrl->m_nTargetSpeed);

}

void _Rover_avoid::draw(void)
{
	this->_AutopilotBase::draw();
	IF_(check()<0);

	string msg = "nObs=" + i2str(m_pDet->size())
			+ ", nTargetSpeed=" + f2str(m_ctrl.m_nTargetSpeed);
	addMsg(msg);

	IF_(!checkWindow());
	Mat* pMat = ((Window*) this->m_pWindow)->getFrame()->m();

	vInt2 cs;
	cs.x = pMat->cols;
	cs.y = pMat->rows;
	Rect r = convertBB<vInt4>(convertBB(m_obs.m_bb, cs));
	rectangle(*pMat, r, Scalar(0,0,255), 3);

}

}
