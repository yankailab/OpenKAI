/*
 *  Created on: Dec 3, 2020
 *      Author: yankai
 */
#include "_InnfosGluon.h"

#ifdef USE_INNFOS

namespace kai
{

_InnfosGluon::_InnfosGluon()
{
	m_ieCheckAlarm.init(100000);
	m_ieSendCMD.init(50000);
	m_ieReadStatus.init(50000);
}

_InnfosGluon::~_InnfosGluon()
{
}

bool _InnfosGluon::init(void* pKiss)
{
	IF_F(!this->_ActuatorBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("tIntCheckAlarm", &m_ieCheckAlarm.m_tInterval);
	pK->v("tIntSendCMD", &m_ieSendCMD.m_tInterval);
	pK->v("tIntReadStatus", &m_ieReadStatus.m_tInterval);
     
	string n;

	return true;
}

bool _InnfosGluon::power(bool bON)
{
    IF_T(bON == m_bPower);

    if(bON)
    {
        IF_F(EstablishConnection());
        IF_F(m_gluon.Initialization());

        m_bPower = true;
    }
    else
    {
        IF_F(m_gluon.Shutdown());

        m_bPower = false;
    }
}

bool _InnfosGluon::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		LOG_E(retCode);
		m_bThreadON = false;
		return false;
	}

	return true;
}

int _InnfosGluon::check(void)
{
    IF__(!m_bPower, -1);
    
	return 0;
}

void _InnfosGluon::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		updateGluon();

		this->autoFPSto();
	}
}

void _InnfosGluon::checkAlarm(void)
{
	IF_(check()<0);
	IF_(!m_ieCheckAlarm.update(m_tStamp));

}

void _InnfosGluon::updateGluon (void)
{
	IF_(check()<0);
	IF_(!m_ieSendCMD.update(m_tStamp));
    
    double pJoint[7];

//    for(int i=0; i<m_vAxis.size(); i++)
//        pJoint[i] = m_vAxis[i].m_p.m_vTarget;
    
//    MoveToTargetJoint(&m_gluon, pJoint);
    
    
    for(int i=0; i<m_vAxis.size(); i++)
        pJoint[i] = m_vAxis[i].m_s.m_vTarget;
    MoveJointIncremental(&m_gluon, pJoint);
    
    
//    m_gluon.GetMaxLineAcceleration(m_maxLinearAccel);
//    m_gluon.GetMaxAngularAcceleration(m_maxAngularAccel);
//    m_gluon.SetMaxLineAcceleration(1000);
//    m_gluon.SetMaxAngularAcceleration(m_maxAngularAccel * 0.5);    
//    m_gluon.SetMaxLineVelocity(100.0);

}

void _InnfosGluon::readStatus(void)
{
	IF_(check()<0);
	IF_(!m_ieReadStatus.update(m_tStamp));
    
    int nAxis = m_gluon.GetAxisNum();
    double pJoint[7];
    m_gluon.GetCurrentJointAngle(pJoint);

    for(int i=0; i<nAxis; i++)
    {
        if(i >= m_vAxis.size())break;
        m_vAxis[i].m_p.m_v = pJoint[i];        
    }
}

void _InnfosGluon::draw(void)
{
	this->_ActuatorBase::draw();
}

}
#endif

