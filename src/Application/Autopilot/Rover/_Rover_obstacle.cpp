#include "_Rover_obstacle.h"

namespace kai
{

_Rover_obstacle::_Rover_obstacle()
{
	m_pAB = NULL;
	m_pDet = NULL;

	m_ctrl.init();
}

_Rover_obstacle::~_Rover_obstacle()
{
}

bool _Rover_obstacle::init(void* pKiss)
{
	IF_F(!this->_AutopilotBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

	string iName;
	iName = "";
	F_ERROR_F(pK->v("_AutopilotBase", &iName));
	m_pAB = (_AutopilotBase*) (pK->parent()->getChildInst(iName));
	NULL_Fl(m_pAB, iName+": not found");

	iName = "";
	F_ERROR_F(pK->v("_DetectorBase", &iName));
	m_pDet = (_DetectorBase*) (pK->root()->getChildInst(iName));
	NULL_Fl(m_pDet, iName+": not found");

	return true;
}

int _Rover_obstacle::check(void)
{
	NULL__(m_pAB, -1);
	NULL__(m_pDet, -1);

	return 0;
}

void _Rover_obstacle::update(void)
{
	this->_AutopilotBase::update();
	IF_(check()<0);
	IF_(!bActive());

	ROVER_CTRL* pCtrl = (ROVER_CTRL*)m_pAB->m_pCtrl;
	m_ctrl.m_vDrive = pCtrl->m_vDrive;

	OBJECT* pO;
	int i=0;
	while((pO = m_pDet->at(i++)) != NULL)
	{
	}

}

void _Rover_obstacle::draw(void)
{
	this->_AutopilotBase::draw();
	IF_(check()<0);
	IF_(!checkWindow());
	Mat* pMat = ((Window*) this->m_pWindow)->getFrame()->m();

}

}
