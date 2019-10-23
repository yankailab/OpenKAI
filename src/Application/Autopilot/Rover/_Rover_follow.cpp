#include "_Rover_follow.h"

namespace kai
{

_Rover_follow::_Rover_follow()
{
	m_pDet = NULL;
	m_pPID = NULL;

	m_target.init();
	m_iClass = 0;
	m_nSpeed = 0.0;

	m_ctrl.init();
	m_pCtrl = (void*)&m_ctrl;
}

_Rover_follow::~_Rover_follow()
{
}

bool _Rover_follow::init(void* pKiss)
{
	IF_F(!this->_AutopilotBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

	pK->v("iClass", &m_iClass);
	pK->v("nSpeed", &m_nSpeed);

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

bool _Rover_follow::start(void)
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

int _Rover_follow::check(void)
{
	NULL__(m_pDet, -1);
	NULL__(m_pPID, -1);
	NULL__(m_pAB, -1);
	NULL__(m_pAB->m_pCtrl, -1);

	return 0;
}

void _Rover_follow::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		this->_AutopilotBase::update();
		updateFollow();

		this->autoFPSto();
	}
}

void _Rover_follow::updateFollow(void)
{
	IF_(check()<0);

	ROVER_CTRL* pCtrl = (ROVER_CTRL*)m_pAB->m_pCtrl;
	if(!bActive())
	{
		m_ctrl = *pCtrl;
		return;
	}

	m_ctrl.m_hdg = pCtrl->m_hdg;
	m_ctrl.m_nSpeed = pCtrl->m_nSpeed;

	OBJECT o;
	o.init();
	OBJECT* pO;
	int i=0;
	while((pO = m_pDet->at(i++)) != NULL)
	{
		IF_CONT(pO->m_topClass != m_iClass);
		if(o.m_topClass >= 0)
		{
			IF_CONT(pO->m_bb.area() < o.m_bb.area());
		}

		o = *pO;
	}

	if(o.m_topClass < 0)
	{
		m_target.init();
		m_ctrl.m_targetHdgOffset = 0.0;
		m_ctrl.m_nTargetSpeed = pCtrl->m_nTargetSpeed;
		return;
	}

	m_target = o;
	m_ctrl.m_targetHdgOffset = m_pPID->update(m_target.m_bb.center().x, 0.5, m_tStamp);
	m_ctrl.m_nTargetSpeed = m_nSpeed;

}

void _Rover_follow::draw(void)
{
	this->_AutopilotBase::draw();
	IF_(check()<0);

	string msg;

	if(!bActive())
	{
		msg = "Inactive";
		addMsg(msg,1);
		return;
	}

	if(m_target.m_topClass < 0)
	{
		msg = "Target not found";
		addMsg(msg,1);
		return;
	}
	else
	{
		string msg = "targetX=" + f2str(m_target.m_bb.center().x) + ", targetHdgOffset=" + f2str(m_ctrl.m_targetHdgOffset);
		addMsg(msg,1);
	}

	IF_(!checkWindow());
	Mat* pMat = ((Window*) this->m_pWindow)->getFrame()->m();

	vInt2 cs;
	cs.x = pMat->cols;
	cs.y = pMat->rows;
	Rect r = convertBB<vInt4>(convertBB(m_target.m_bb, cs));
	rectangle(*pMat, r, Scalar(0,255,255), 3);

}

}
