/*
 * _SortingArm.cpp
 *
 *  Created on: May 28, 2019
 *      Author: yankai
 */

#include "../../RobotArm/Cartesian/_SortingArm.h"

#ifdef USE_OPENCV

namespace kai
{

_SortingArm::_SortingArm()
{
	m_pCS = NULL;
	m_nClass = 0;
	m_iLastState = -1;

	m_pSeq = NULL;
	m_classFlag = 0;
	m_rGripX.init();
	m_vRoiX.x = 0.0;
	m_vRoiX.y = 1.0;
	m_rGripY.init();
	m_vRoiY.x = 0.0;
	m_vRoiY.y = 1.0;
	m_rGripZ.init();
	m_actuatorX = "";
	m_actuatorZ = "";
	m_tO.init();
}

_SortingArm::~_SortingArm()
{
}

bool _SortingArm::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("actuatorX",&m_actuatorX);
	pK->v("actuatorZ",&m_actuatorZ);

	pK->v("gripX", &m_rGripX);
	pK->v("vRoiX", &m_vRoiX);
	pK->v("gripY", &m_rGripY);
	pK->v("vRoiY", &m_vRoiY);
	pK->v("gripZ", &m_rGripZ);
	pK->a("vDropPos", &m_vDropPos);

	string iName;
	int i;

	vector<int> vClass;
	m_nClass = pK->a("classList", &vClass);
	for (i = 0; i < m_nClass; i++)
		m_classFlag |= (1 << vClass[i]);

	vector<string> vActuator;
	pK->a("vActuator", &vActuator);
	for (i = 0; i < vActuator.size(); i++)
	{
		iName = vActuator[i];
		_ActuatorBase* pA = (_ActuatorBase*) (pK->getInst(iName));
		IF_Fl(!pA, iName + " not found");
		m_vAB.push_back(pA);
	}

	iName = "";
	F_ERROR_F(pK->v("_Sequencer", &iName));
	m_pSeq = (_Sequencer*) (pK->getInst(iName));
	IF_Fl(!m_pSeq, iName + " not found");

	iName = "";
	F_ERROR_F(pK->v("_SortingCtrlServer", &iName));
	m_pCS = (_SortingCtrlServer*) (pK->getInst(iName));
	IF_Fl(!m_pCS, iName + " not found");

	return true;
}

bool _SortingArm::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _SortingArm::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		updateArm();

		this->autoFPSto();
	}
}

int _SortingArm::check(void)
{
	NULL__(m_pCS, -1);

	return 0;
}

void _SortingArm::updateArm(void)
{
	IF_(check() < 0);

	if(m_pCS->m_iState == SORT_STATE_OFF)
	{
		m_tO.init();
		m_pSeq->off();
		m_iLastState = m_pCS->m_iState;

		for(int i=0; i<m_vAB.size(); i++)
		{
			_ActuatorBase* pA = m_vAB[i];
			pA->gotoOrigin();
			sleep(1);
		}

		return;
	}

	if(m_pCS->m_iState != m_iLastState)
	{
		m_tO.init();
		m_pSeq->on();
		m_iLastState = m_pCS->m_iState;
	}

	vFloat4 vP,vS;
	vP.init(-1.0);
	vS.init(-1.0);
	vS.x = 1.0;

	float x,y;

	_Object* pO;
	string cAction = m_pSeq->getCurrentActionName();

	if (cAction == "prepareX")
	{
		if(m_tO.getTopClass() < 0)
		{
			m_tO.init();
			m_pSeq->on();
			return;
		}

		float spd = m_pCS->m_cSpeed * ((float) m_dTime) * 1e-6;
		m_tO.setY(m_tO.getY() + spd);
		y = m_tO.getY();

		IF_(y < m_rGripY.x);
		if(y < m_rGripY.y)
			m_pSeq->m_tResume = 0;
		else	//out of range, simply reset
			m_pSeq->on();

		m_tO.init();
		return;
	}

	if (cAction == "standby")
	{
		int i = 0;
		while((pO = m_pCS->m_pU->get(i++)))
		{
			x = pO->getX();
			y = pO->getY();

			IF_CONT(x < m_vRoiX.x);
			IF_CONT(x > m_vRoiX.y);
			IF_CONT(y < m_vRoiY.x);
			IF_CONT(y > m_vRoiY.y);

			m_tO = *pO;
			pO->setY(pO->getY() + m_pCS->m_cLen);

			SEQ_ACTION* pAction;
			SEQ_ACTUATOR* pSA;

			//prepare X position
			pAction = m_pSeq->getAction("prepareX");
			IF_CONT(!pAction);

			//X horizontal
			pSA = pAction->getActuator(m_actuatorX);
			IF_CONT(!pSA);

			vP.init(-1.0);
			vP.x = constrain(x, m_vRoiX.x, m_vRoiX.y);
			vP.x = map<float>(vP.x,
					m_vRoiX.x,
					m_vRoiX.y,
					0.0,1.0);
			vP.x = map<float>(1.0 - vP.x,
					0.0,
					1.0,
					m_rGripX.x,
					m_rGripX.y);
			vP.x = constrain<float>(vP.x, m_rGripX.x, m_rGripX.y);
			pSA->setTarget(vP, vS);

			//catch position
			pAction = m_pSeq->getAction("descent");
			IF_CONT(!pAction);

			//Z vertical
			pSA = pAction->getActuator(m_actuatorZ);
			IF_CONT(!pSA);

			vP.init(-1.0);
			vP.x = (pO->getZ() - m_rGripZ.x) / m_rGripZ.len();
			pSA->setTarget(vP, vS);

			//dest
			pAction = m_pSeq->getAction("move");
			IF_CONT(!pAction);

			pSA = pAction->getActuator(m_actuatorX);
			IF_CONT(!pSA);

			vP.init(-1.0);
			vP.x = m_vDropPos[pO->getTopClass()];
			pSA->setTarget(vP, vS);

			m_pSeq->m_tResume = 0;
			return;
		}
	}
}

void _SortingArm::draw(void)
{
	this->_ThreadBase::draw();

	IF_(!checkWindow());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();

	int iL = m_vRoiX.x * pMat->cols;
	int iR = m_vRoiX.y * pMat->cols;

	line(*pMat, Point(iL, 0),
				Point(iL, pMat->rows),
				Scalar(0,255,0));

	line(*pMat, Point(iR, 0),
				Point(iR, pMat->rows),
				Scalar(0,255,0));

}

}
#endif
