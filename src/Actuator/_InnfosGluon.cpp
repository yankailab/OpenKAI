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
	m_gluon.Shutdown();
}

bool _InnfosGluon::init(void* pKiss)
{
	IF_F(!this->_ActuatorBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("tIntCheckAlarm", &m_ieCheckAlarm.m_tInterval);
	pK->v("tIntSendCMD", &m_ieSendCMD.m_tInterval);
	pK->v("tIntReadStatus", &m_ieReadStatus.m_tInterval);
    
    IF_F(EstablishConnection());
    //power on the actuator
	IF_F(m_gluon.Initialization());
 
	string n;

	return true;
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

void _InnfosGluon::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		if(m_bFeedback)
		{
//			checkAlarm();
//			readStatus();
		}
		sendCMD();

		this->autoFPSto();
	}
}

int _InnfosGluon::check(void)
{
	return 0;
}

void _InnfosGluon::checkAlarm(void)
{
	IF_(check()<0);
	IF_(!m_ieCheckAlarm.update(m_tStamp));


}

void _InnfosGluon::sendCMD(void)
{
	IF_(check()<0);
	IF_(!m_ieSendCMD.update(m_tStamp));

    m_gluon.GetMaxLineAcceleration(m_maxLinearAccel);
    m_gluon.GetMaxAngularAcceleration(m_maxAngularAccel);

    m_gluon.SetMaxLineAcceleration(1000);
    m_gluon.SetMaxAngularAcceleration(m_maxAngularAccel * 0.5);
    
    m_gluon.SetMaxLineVelocity(100.0);
    
    
    
    
    
    
    //获取机械臂的轴数，6轴机械臂==6
	int axis_num=m_gluon.GetAxisNum();
	double target_joint[axis_num];

	target_joint[0] = 0.0;
	target_joint[1] = 0.0;
	target_joint[2] = -110*pi/180;
	target_joint[3] = -64*pi/180;
	target_joint[4] = 105*pi/180;
	target_joint[5] = 0.0; 

	//以关节运动的方式从当前点移动到目标点，本例为指定的一个点，关节角度如上所示
	//可通过函数robot.SetMaxJointVelocity(const double vel [ ])，设置最大速度，默认为10*pi/180弧度/秒
	MoveToTargetJoint(&m_gluon, target_joint);

	double inc_joint[axis_num];
	for (int i=0;i<axis_num;i++)
	{
		inc_joint[i] = 20*pi/180;
	}

	//以关节增量运动的方式，本例每轴移动正20度
	MoveJointIncremental(&m_gluon,inc_joint);

	for (int i=0;i<axis_num;i++)
	{
		inc_joint[i] = -20*pi/180;
	}

	//以关节增量运动的方式，本例每轴移动负20度
	MoveJointIncremental(&m_gluon, inc_joint);
    
    return;
    
    
    
    
    double target_pose [ 7 ];
	double aux_pose [ 7 ];

	//获取当前点的位姿
	m_gluon.GetCurrentPose(target_pose);
	m_gluon.GetCurrentPose(aux_pose);

//	aux_pose[1] += 100;
	target_pose[1] += 100;
    MoveLineToTargetPose(&m_gluon, target_pose);


//    TranslateZaxis(&m_gluon, -100);
//	TranslateZaxis(&m_gluon, 100);
//	TranslateXaxis(&m_gluon, 100);
//	TranslateYaxis(&m_gluon, 100);

//	TranslateYaxis(&m_gluon, -100);
//	TranslateYaxis(&m_gluon, 100);
	

//	RotateXAxis(&m_gluon, -10*pi/180);
//	RotateXAxis(&m_gluon, 10*pi/180);

//  RotateYAxis(&m_gluon, -10*pi/180);
//	RotateYAxis(&m_gluon, 10*pi/180);

//	RotateZAxis(&m_gluon, -10*pi/180);
//    RotateZAxis(&m_gluon, 10*pi/180);
}

void _InnfosGluon::readStatus(void)
{
	IF_(check()<0);
	IF_(!m_ieReadStatus.update(m_tStamp));

    
}

void _InnfosGluon::draw(void)
{
	this->_ActuatorBase::draw();
}

}
#endif

