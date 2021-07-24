/*
 *  Created on: June 22, 2020
 *      Author: yankai
 */
#include "_HYMCU_RS485.h"

namespace kai
{

_HYMCU_RS485::_HYMCU_RS485()
{
	m_pMB = NULL;
	m_iSlave = 1;
	m_iData = 0;
	m_dpr = 1;
	m_pA = NULL;
	m_dInit = 20;
	m_cmdInt = 50000;

	m_ieReadStatus.init(50000);
}

_HYMCU_RS485::~_HYMCU_RS485()
{
}

bool _HYMCU_RS485::init(void* pKiss)
{
	IF_F(!this->_ActuatorBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("iSlave", &m_iSlave);
	pK->v("iData", &m_iData);
	pK->v("dpr", &m_dpr);
	pK->v("dInit", &m_dInit);
	pK->v("cmdInt", &m_cmdInt);
	pK->v("tIntReadStatus", &m_ieReadStatus.m_tInterval);

	Kiss* pKa = pK->child("addr");
	if(pKa)
	{
		pKa->v("setDPR", &m_addr.m_setDPR);
		pKa->v("setDist", &m_addr.m_setDist);
		pKa->v("setDir", &m_addr.m_setDir);
		pKa->v("setSpd", &m_addr.m_setSpd);
		pKa->v("setAcc", &m_addr.m_setAcc);
		pKa->v("bComplete", &m_addr.m_bComplete);
		pKa->v("readStat", &m_addr.m_readStat);
		pKa->v("run", &m_addr.m_run);
		pKa->v("stop", &m_addr.m_stop);
		pKa->v("resPos", &m_addr.m_resPos);
	}

	m_pA = &m_vAxis[0];

	string n;
	n = "";
	F_ERROR_F(pK->v("_Modbus", &n));
	m_pMB = (_Modbus*) (pK->getInst(n));
	IF_Fl(!m_pMB, n + " not found");

	return true;
}

bool _HYMCU_RS485::start(void)
{
    NULL_F(m_pT);
	return m_pT->start(getUpdate, this);
}

int _HYMCU_RS485::check(void)
{
	NULL__(m_pMB,-1);
	IF__(!m_pMB->bOpen(),-1);
	NULL__(m_pA,-1);

	return this->_ActuatorBase::check();
}

void _HYMCU_RS485::update(void)
{
//	setSlaveID(2);
//	saveData();

//	while(!initPos());

	while(m_pT->bRun())
	{
		m_pT->autoFPSfrom();

		readStatus();
		updateMove();

		m_pT->autoFPSto();
	}
}

void _HYMCU_RS485::updateMove(void)
{
	IF_(check()<0);

	if(m_bf.b(ACT_BF_STOP))
	{
		stop();
		return;
	}

//	IF_(!bComplete()); //Moving

	IF_(!setPos());
	IF_(!setSpeed());
	IF_(!setAccel());
	run();
}

bool _HYMCU_RS485::setDistPerRound(int32_t dpr)
{
	IF_F(check()<0);

	uint16_t pB[2];
	pB[0] = HIGH16(dpr);
	pB[1] = LOW16(dpr);
	IF_F(m_pMB->writeRegisters(m_iSlave, m_addr.m_setDPR, 2, pB) != 2);
	m_pT->sleepT (m_cmdInt);

	return true;
}

bool _HYMCU_RS485::setPos(void)
{
	IF_F(check()<0);

//	int32_t step = m_pA->m_p.m_vTarget - m_pA->m_p.m_v;
	int32_t step = m_pA->m_p.m_vTarget;
	IF_F(step==0);
	int32_t ds = abs(step);

	uint16_t pB[2];
	pB[0] = HIGH16(ds);
	pB[1] = LOW16(ds);
	IF_F(m_pMB->writeRegisters(m_iSlave, m_addr.m_setDist, 2, pB) != 2);
	m_pT->sleepT (m_cmdInt);

	pB[0] = (step > 0)?0:1;
	IF_F(m_pMB->writeRegisters(m_iSlave, m_addr.m_setDir, 1, pB) != 1);
	m_pT->sleepT (m_cmdInt);

	return true;
}

bool _HYMCU_RS485::setSpeed(void)
{
	IF_F(check()<0);

	uint16_t b = m_pA->m_s.m_vTarget;
	IF_F(m_pMB->writeRegisters(m_iSlave, m_addr.m_setSpd, 1, &b) != 1);
	m_pT->sleepT (m_cmdInt);

	return true;
}

bool _HYMCU_RS485::setAccel(void)
{
	IF_F(check()<0);

	uint16_t b = m_pA->m_a.m_vTarget;
	IF_F(m_pMB->writeRegisters(m_iSlave, m_addr.m_setAcc, 1, &b) != 1);
	m_pT->sleepT (m_cmdInt);

	return true;
}

bool _HYMCU_RS485::setBrake(void)
{
	IF_F(check()<0);

	return true;
}

bool _HYMCU_RS485::setSlaveID(uint16_t iSlave)
{
	IF_F(check()<0);

	IF_F(m_pMB->writeRegisters(m_iSlave, m_addr.m_setSlaveID, 1, &iSlave) != 1);
	m_pT->sleepT (m_cmdInt);

	return true;
}

bool _HYMCU_RS485::saveData(void)
{
	IF_F(check()<0);

	IF_F(m_pMB->writeBit(m_iSlave, m_addr.m_saveData, true) != 1);
	m_pT->sleepT (m_cmdInt);
	return true;
}

bool _HYMCU_RS485::run(void)
{
	IF_F(check()<0);

	IF_F(m_pMB->writeBit(m_iSlave, m_addr.m_run, true) != 1);
	m_pT->sleepT (m_cmdInt);
	return true;
}

bool _HYMCU_RS485::stop(void)
{
	IF_F(check()<0);

	IF_F(m_pMB->writeBit(m_iSlave, m_addr.m_stop, true) != 1);
	m_pT->sleepT (m_cmdInt);
	return true;
}

bool _HYMCU_RS485::resetPos(void)
{
	IF_F(check()<0);

	IF_F(m_pMB->writeBit(m_iSlave, m_addr.m_resPos, true) != 1);
	m_pT->sleepT (m_cmdInt);
	return true;
}

bool _HYMCU_RS485::initPos(void)
{
	IF_F(check()<0);
	IF_F(!setDistPerRound(m_dpr));

	uint16_t pB[2];
	int32_t ds = abs(m_dInit);
	pB[0] = HIGH16(ds);
	pB[1] = LOW16(ds);
	IF_F(m_pMB->writeRegisters(m_iSlave, m_addr.m_setDist, 2, pB) != 2);
	m_pT->sleepT (m_cmdInt);

	pB[0] = (m_dInit > 0)?0:1;
	IF_F(m_pMB->writeRegisters(m_iSlave, m_addr.m_setDir, 1, pB) != 1);
	m_pT->sleepT (m_cmdInt);

	IF_F(!setSpeed());
	IF_F(!setAccel());
	IF_F(!run());

	while(!bComplete());
	while(!resetPos());

	return true;
}

bool _HYMCU_RS485::bComplete(void)
{
	IF_F(check()<0);

	uint16_t b;
	int r = m_pMB->readRegisters(m_iSlave, m_addr.m_bComplete, 1, &b);
	IF_F(r != 1);
	m_pT->sleepT (m_cmdInt);

	return (b==1)?true:false;
}

bool _HYMCU_RS485::readStatus(void)
{
	IF_F(check()<0);
	IF_T(!m_ieReadStatus.update(m_pT->getTfrom()));

	uint16_t pB[2];
	int r = m_pMB->readRegisters(m_iSlave, m_addr.m_readStat, 2, pB);
	IF_F(r != 2);
	m_pT->sleepT (m_cmdInt);

//	int p = MAKE32(pB[0], pB[1]);
	int16_t p = pB[1];
	m_pA->m_p.m_v = p;

	return true;
}

}
