/*
 * _SortingArm.cpp
 *
 *  Created on: May 28, 2019
 *      Author: yankai
 */

#include "_SortingArm.h"

namespace kai
{

_SortingArm::_SortingArm()
{
	m_pSW = NULL;
	m_pC = NULL;
	m_nClass = 0;

	m_pSeq = NULL;
	m_classFlag = 0;
	m_rGripX.init();
	m_rGripY.init();
	m_rGripZ.init();
	m_actuatorX = "";
	m_actuatorZ = "";
	m_iROI = 0;

}

_SortingArm::~_SortingArm()
{
}

bool _SortingArm::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK, actuatorX);
	KISSm(pK, actuatorZ);
	KISSm(pK, iROI);

	pK->v("gripX1", &m_rGripX.x);
	pK->v("gripX2", &m_rGripX.y);
	pK->v("gripY1", &m_rGripY.x);
	pK->v("gripY2", &m_rGripY.y);
	pK->v("gripZ1", &m_rGripZ.x);
	pK->v("gripZ2", &m_rGripZ.y);
	pK->array("dropPos", m_pDropPos, SB_N_CLASS);

	int pClass[SB_N_CLASS];
	m_nClass = pK->array("class", pClass, SB_N_CLASS);
	for (int i = 0; i < m_nClass; i++)
		m_classFlag |= (1 << pClass[i]);

	string iName;

	iName = "";
	F_ERROR_F(pK->v("_Sequencer", &iName));
	m_pSeq = (_Sequencer*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pSeq, iName + " not found");

	iName = "";
	F_ERROR_F(pK->v("_SlideWindow", &iName));
	m_pSW = (_DetectorBase*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pSW, iName + " not found");

	iName = "";
	F_ERROR_F(pK->v("_DNNclassifier", &iName));
	m_pC = (_DetectorBase*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pC, iName + " not found");

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
	NULL__(m_pSW, -1);
	NULL__(m_pC, -1);

	return 0;
}

void _SortingArm::updateArm(void)
{
	IF_(check() < 0);

	OBJECT* pO;
	string cAction = m_pSeq->getCurrentActionName();

	if (cAction == "standby")
	{
		int i = 0;
		while((pO=m_pSW->at(i++)))
		{
			IF_CONT(!(m_classFlag & (1 << pO->m_topClass)));
			IF_CONT(pO->m_bb.midY() < m_rGripY.x);
			IF_CONT(pO->m_bb.midY() > m_rGripY.y);
			pO->m_bb.y += FLT_MAX;
			pO->m_bb.w += FLT_MAX;

			SEQ_ACTION* pAction;
			SEQ_ACTUATOR* pSA;

			pAction = m_pSeq->getAction("descent");
			IF_CONT(!pAction);

			pSA = pAction->getActuator(m_actuatorX);
			IF_CONT(pSA);
			pSA->setTarget((1.0 - pO->m_bb.midX()) * m_rGripX.len() + m_rGripX.x, 1.0);

			pSA = pAction->getActuator(m_actuatorZ);
			IF_CONT(pSA);
			pSA->setTarget((pO->m_dist - m_rGripZ.x) / m_rGripZ.len(), 1.0);

			pAction = m_pSeq->getAction("move");
			IF_CONT(!pAction);

			pSA = pAction->getActuator(m_actuatorX);
			IF_CONT(pSA);
			pSA->setTarget(m_pDropPos[pO->m_topClass], 1.0);

			m_pSeq->wakeUp();
			return;
		}
	}
	else if (cAction == "verify")
	{
		OBJECT o = *m_pC->at(m_iROI);





	}
}

bool _SortingArm::draw(void)
{
	IF_F(!this->_ThreadBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();

	return true;
}

bool _SortingArm::console(int& iY)
{
	IF_F(!this->_ThreadBase::console(iY));

	string msg;

	return true;
}

}
