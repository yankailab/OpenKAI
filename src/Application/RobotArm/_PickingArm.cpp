/*
 * _PickingArm.cpp
 *
 *  Created on: June 30, 2020
 *      Author: yankai
 */

#include "_PickingArm.h"

#ifdef USE_OPENCV

namespace kai
{

_PickingArm::_PickingArm()
{
	m_pA = NULL;
	m_pG = NULL;
	m_pD = NULL;
	m_pU = NULL;

	m_baseAngle = 0.0;
	m_vP.init(0.5,0.5,0.0);
	m_vPtarget.init(0.5,0.5,0.0);
	m_vZrange.init(0.0, 0.06, 0.15, 30.0);
	m_zSpeed = 0.2;
	m_zrK = 1.0;

	m_pXpid = NULL;
	m_pYpid = NULL;
	m_pZpid = NULL;

	m_vS.init(0.5);
	m_vR.init(0.5);

	m_vPrecover.init(0.0, 0.0, 0.0);
	m_vPdeliver.init(-200, 300.0, 0.0);
	m_vPdescend.init(-200, 300.0, -100.0);

	m_oTstamp = 0;
	m_oTimeout = USEC_1SEC;
}

_PickingArm::~_PickingArm()
{
}

bool _PickingArm::init(void *pKiss)
{
	IF_F(!this->_StateBase::init(pKiss));
	Kiss *pK = (Kiss*) pKiss;

	pK->v("vPtarget", &m_vPtarget);
	pK->v("vZrange", &m_vZrange);
	pK->v("zSpeed", &m_zSpeed);
	pK->v("zrK", &m_zrK);
	pK->v("vPrecover", &m_vPrecover);
	pK->v("vPdeliver", &m_vPdeliver);
	pK->v("vPdescend", &m_vPdescend);
	pK->v("oTimeout", &m_oTimeout);

	Kiss *pClass = pK->child("class");
	NULL_Fl(pClass, "class not found");

	int i = 0;
	while (1)
	{
		Kiss *pC = pClass->child(i++);
		if (pC->empty())
			break;

		PICKINGARM_CLASS pc;
		pc.init();
		pC->v("iClass", &pc.m_iClass);

		m_vClass.push_back(pc);
	}

	IF_F(!m_pSC);
	m_iState.EXTERNAL = m_pSC->getStateIdxByName ("EXTERNAL");
	m_iState.RECOVER = m_pSC->getStateIdxByName ("RECOVER");
	m_iState.FOLLOW = m_pSC->getStateIdxByName ("FOLLOW");
	m_iState.ASCEND = m_pSC->getStateIdxByName ("ASCEND");
	m_iState.DELIVER = m_pSC->getStateIdxByName ("DELIVER");
	m_iState.DESCEND = m_pSC->getStateIdxByName ("DESCEND");
	m_iState.DROP = m_pSC->getStateIdxByName ("DROP");
	IF_F(!m_iState.bValid());

	string n;

	n = "";
	F_ERROR_F(pK->v("_ActuatorBase", &n));
	m_pA = (_ActuatorBase*) (pK->getInst(n));
	NULL_Fl(m_pA, n + " not found");

	n = "";
	F_ERROR_F(pK->v("_StepperGripper", &n));
	m_pG = (_StepperGripper*)pK->getInst(n);
	NULL_Fl(m_pG, n + ": not found");

	n = "";
	F_ERROR_F(pK->v("_Universe", &n));
	m_pU = (_Universe*) (pK->getInst(n));
	NULL_Fl(m_pU, n + " not found");

	n = "";
	F_ERROR_F(pK->v("_DistSensorBase", &n));
	m_pD = (_DistSensorBase*) (pK->getInst(n));
	NULL_Fl(m_pD, n + " not found");

	n = "";
	F_ERROR_F(pK->v("PIDx", &n));
	m_pXpid = ( PID*) (pK->getInst(n));
	NULL_Fl(m_pXpid, n + " not found");

	n = "";
	F_ERROR_F(pK->v("PIDy", &n));
	m_pYpid = ( PID*) (pK->getInst(n));
	NULL_Fl(m_pYpid, n + " not found");

	n = "";
	F_ERROR_F(pK->v("PIDz", &n));
	m_pZpid = ( PID*) (pK->getInst(n));
	NULL_Fl(m_pZpid, n + " not found");

	return true;
}

bool _PickingArm::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdate, this);
	if (retCode != 0)
	{
		m_bThreadON = false;
		return false;
	}

	return true;
}

int _PickingArm::check(void)
{
	NULL__(m_pA, -1);
	NULL__(m_pG, -1);
	NULL__(m_pD, -1);
	NULL__(m_pU, -1);
	NULL__(m_pXpid, -1);
	NULL__(m_pYpid, -1);
	NULL__(m_pZpid, -1);

	return this->_StateBase::check();
}

void _PickingArm::update(void)
{
	while(m_pT->bRun())
	{
		m_pT->autoFPSfrom();

		this->_StateBase::update();
		updateArm();

		m_pT->autoFPSto();
	}
}

void _PickingArm::updateArm(void)
{
	IF_(check() < 0);

	int iM = m_pSC->getStateIdx();
	bool bTransit = false;

	if(iM == m_iState.EXTERNAL)
	{
		external();
	}
	else if(iM == m_iState.RECOVER)
	{
		bTransit = recover();
	}
	else if(iM == m_iState.FOLLOW)
	{
		bTransit = follow();
	}
	else if(iM == m_iState.ASCEND)
	{
		bTransit = ascend();
	}
	else if(iM == m_iState.DELIVER)
	{
		bTransit = deliver();
	}
	else if(iM == m_iState.DESCEND)
	{
		bTransit = descend();
	}
	else if(iM == m_iState.DROP)
	{
		bTransit = drop();
	}
	else
	{
		stop();
	}

	if(bTransit)
		m_pSC->transit();
}

void _PickingArm::stop(void)
{
	m_pA->setStarget(0, 0.5);
	m_pA->setStarget(1, 0.5);
	m_pA->setStarget(2, 0.5);
	m_pA->setStarget(6, 0.5);
	m_pA->setStarget(7, 0.5);
	m_pA->setStarget(8, 0.5);
}

void _PickingArm::external(void)
{
	m_pA->setStarget(0, m_vS.x);
	m_pA->setStarget(1, m_vS.y);
	m_pA->setStarget(2, m_vS.z);
	m_pA->setStarget(6, m_vR.x);
	m_pA->setStarget(7, m_vR.y);
	m_pA->setStarget(8, m_vR.z);
}

bool _PickingArm::recover(void)
{
	m_pA->atomicFrom();
	m_pA->setPtarget(0, m_vPrecover.x);
	m_pA->setPtarget(1, m_vPrecover.y);
	m_pA->setPtarget(2, m_vPrecover.z);
	m_pA->atomicTo();

	IF_F(!m_pA->bComplete(0));
	IF_F(!m_pA->bComplete(1));
	IF_F(!m_pA->bComplete(2));

	return true;
}

bool _PickingArm::follow(void)
{
	m_vP.z = m_pD->d(0);

	if(m_vP.z < m_vZrange.x)
	{
		//invalid reading of Z, go back to origin
		recover();
		return false;
	}
	else if(m_vP.z < m_vZrange.y)
	{
		//stop and gripper closed
		stop();
		IF_T(!m_pG->bGrip());
		m_pG->grip(false);
		return false;
	}
	else if(m_vP.z < m_vZrange.z)
	{
		//gripper start to close
		m_pG->grip(false);
		return false;
	}
	else if(m_vP.z < m_vZrange.w)
	{
		//keep on getting closer even no target is seen any more
		speed(vFloat3(0.5, 0.5, 0.5+m_zSpeed));
		return false;
	}

	//look for target
	_Object* tO = findTarget();

	if(!tO)
	{
		//no target is seen, ascend the arm
		//TODO: set speed
		recover();
		return false;
	}

	m_baseAngle = -m_pA->getP(3);
	float rad = m_baseAngle * DEG_2_RAD;
	float s = sin(rad);
	float c = cos(rad);

	vFloat3 vP = tO->getPos();
	float x = vP.x - m_vPtarget.x;
	float y = vP.y - m_vPtarget.y;
	float r = sqrt(x*x + y*y);
	m_vP.x = x*c - y*s + m_vPtarget.x;
	m_vP.y = x*s + y*c + m_vPtarget.y;

	m_vS.y = 0.5 + m_pXpid->update(m_vP.x, m_vPtarget.x, m_tStamp);
	m_vS.x = 0.5 + m_pYpid->update(m_vP.y, m_vPtarget.y, m_tStamp);
	m_vS.z = 0.5 + m_pZpid->update(m_vP.z, m_vPtarget.z, m_tStamp) * constrain(1.0 - r*m_zrK, 0.0, 1.0);
	speed(m_vS);

	return false;
}

_Object* _PickingArm::findTarget(void)
{
	_Object *pO;
	_Object *tO = NULL;
//	float topProb = 0.0;
	float rSqr = FLT_MAX;
	int i = 0;
	while ((pO = m_pU->get(i++)) != NULL)
	{

		vFloat3 p = pO->getPos();
		float x = p.x - m_vPtarget.x;
		float y = p.y - m_vPtarget.y;
		float r = x*x + y*y;
		IF_CONT(r > rSqr);
//		IF_CONT(pO->getTopClassProb() < topProb);
		IF_CONT(!bTargetClass(pO->getTopClass()));

		tO = pO;
//		topProb = pO->getTopClassProb();
		rSqr = r;
	}

	if(tO)
	{
		m_o = *tO;
		m_oTstamp = m_tStamp;
		return &m_o;
	}

	if(m_tStamp - m_oTstamp < m_oTimeout)
	{
		return &m_o;
	}

	return NULL;
}

bool _PickingArm::bTargetClass(int iClass)
{
	IF_T(m_vClass.empty());

	for(PICKINGARM_CLASS c : m_vClass)
	{
		IF_CONT(c.m_iClass != iClass);
		return true;
	}

	return false;
}

void _PickingArm::speed(const vFloat3& vS)
{
	m_pA->setStarget(0, vS.x);
	m_pA->setStarget(1, vS.y);
	m_pA->setStarget(2, vS.z);
}

bool _PickingArm::ascend(void)
{
	m_pA->atomicFrom();
	m_pA->setPtarget(0, m_vPrecover.x);
	m_pA->setPtarget(1, m_vPrecover.y);
	m_pA->setPtarget(2, m_vPrecover.z);
	m_pA->atomicTo();

	IF_F(!m_pA->bComplete(0));
	IF_F(!m_pA->bComplete(1));
	IF_F(!m_pA->bComplete(2));

	return true;
}

bool _PickingArm::deliver(void)
{
	m_pA->atomicFrom();
	m_pA->setPtarget(0, m_vPdeliver.x);
	m_pA->setPtarget(1, m_vPdeliver.y);
	m_pA->setPtarget(2, m_vPdeliver.z);
	m_pA->atomicTo();

	IF_F(!m_pA->bComplete(0));
	IF_F(!m_pA->bComplete(1));
	IF_F(!m_pA->bComplete(2));
	return true;
}

bool _PickingArm::descend(void)
{
	m_pA->atomicFrom();
	m_pA->setPtarget(0, m_vPdescend.x);
	m_pA->setPtarget(1, m_vPdescend.y);
	m_pA->setPtarget(2, m_vPdescend.z);
	m_pA->atomicTo();

	IF_F(!m_pA->bComplete(2));
	return true;
}

bool _PickingArm::drop(void)
{
	m_pG->grip(true);

	IF_F(!m_pG->bGrip());
	return true;
}

void _PickingArm::move(vFloat3& vM)
{
	m_vS = vM;
}

void _PickingArm::rotate(vFloat3& vR)
{
	m_vR = vR;
}

void _PickingArm::grip(bool bOpen)
{
	IF_(check() < 0);

	m_pG->grip(bOpen);
}

void _PickingArm::draw(void)
{
	this->_StateBase::draw();

	addMsg("vS = (" + f2str(m_vS.x) + ", " + f2str(m_vS.y) + ", " + f2str(m_vS.z) + ")");
	addMsg("vR = (" + f2str(m_vR.x) + ", " + f2str(m_vR.y) + ", " + f2str(m_vR.z) + ")");
	addMsg("vP = (" + f2str(m_vP.x) + ", " + f2str(m_vP.y) + ", " + f2str(m_vP.z) + ")");
	addMsg("vPtarget = (" + f2str(m_vPtarget.x) + ", " + f2str(m_vPtarget.y) + ", " + f2str(m_vPtarget.z) + ")");
	addMsg("baseAngle = " + f2str(m_baseAngle));

	IF_(!checkWindow());
	Mat* pM = ((Window*) this->m_pWindow)->getFrame()->m();
	Point pC = Point(m_vP.x * pM->cols, m_vP.y * pM->rows);
	circle(*pM, pC, 5.0, Scalar(255, 255, 0), 2);
}

}
#endif
