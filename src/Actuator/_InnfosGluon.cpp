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
			checkAlarm();
			readStatus();
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
//	IF_(check()<0);
//	IF_(!m_ieSendCMD.update(m_tStamp));

    
	IF_(EstablishConnection());
	ActuatorGroup robot;
    //power on the actuator
	IF_(robot.Initialization());
    
    //获取机械臂的轴数，6轴机械臂==6
	int axis_num=robot.GetAxisNum();
	double target_joint[axis_num];

	target_joint[0] = 0.0;
	target_joint[1] = 0.0;
	target_joint[2] = -110*pi/180;
	target_joint[3] = -64*pi/180;
	target_joint[4] = 105*pi/180;
	target_joint[5] = 0.0; 

	//以关节运动的方式从当前点移动到目标点，本例为指定的一个点，关节角度如上所示
	//可通过函数robot.SetMaxJointVelocity(const double vel [ ])，设置最大速度，默认为10*pi/180弧度/秒
	if(MoveToTargetJoint(&robot,target_joint))
	{
		return;
	}

	double inc_joint[axis_num];
	for (int i=0;i<axis_num;i++)
	{
		inc_joint[i] = 20*pi/180;
	}

	//以关节增量运动的方式，本例每轴移动正20度
	if(MoveJointIncremental(&robot,inc_joint))
	{
		return;
	}

	for (int i=0;i<axis_num;i++)
	{
		inc_joint[i] = -20*pi/180;
	}
	//以关节增量运动的方式，本例每轴移动负20度
	if(MoveJointIncremental(&robot,inc_joint))
	{
		return;
	}


	double target_pose [ 7 ];
	double aux_pose [ 7 ];

	//获取当前点的位姿
	robot.GetCurrentPose(target_pose);
	robot.GetCurrentPose(aux_pose);

	aux_pose[0] += 25;
	target_pose[1] += 25;

	//以直线的运动方式从当前点移动到目标点
	if(MoveLineToTargetPose(&robot,aux_pose))
	{
		return;
	}

	//沿机器人基系x轴平移-25mm
	if(TranslateXaxis(&robot, -25))
	{
		return;
	}

	//沿机器人基系x轴平移25mm
	if(TranslateXaxis(&robot, 25))
	{
		return;
	}

	//沿机器人基系y轴平移-25mm
	if(TranslateYaxis(&robot, -25))
	{
		return;
	}

	//沿机器人基系y轴平移25mm
	if(TranslateYaxis(&robot, 25))
	{
		return;
	}

	//沿机器人基系z轴平移-25mm
	if(TranslateZaxis(&robot, -25))
	{
		return;
	}

	//沿机器人基系z轴平移25mm
	if(TranslateZaxis(&robot, 25))
	{
		return;
	}

	//绕机器人基系x轴旋转-20度
	if(RotateXAxis(&robot, -20*pi/180))
	{
		return;
	}

	//绕机器人基系x轴旋转20度
	if(RotateXAxis(&robot, 20*pi/180))
	{
		return;
	}

	//绕机器人基系y轴旋转-20度
	if(RotateYAxis(&robot, -20*pi/180))
	{
		return;
	}

	//绕机器人基系y轴旋转20度
	if(RotateYAxis(&robot, 20*pi/180))
	{
		return;
	}

	//绕机器人基系z轴旋转-20度
	if(RotateZAxis(&robot, -20*pi/180))
	{
		return;
	}

	//绕机器人基系z轴旋转20度
	if(RotateZAxis(&robot, 20*pi/180))
	{
		return;
	}

	//以圆弧的运动方式，从当前点经过辅助点移动到目标点
	if(MoveArcToTargetPose(&robot,aux_pose,target_pose))
	{
		return;
	}

	//以关节运动的方式从当前点移动到目标点，本例目标点为零点
	memset(target_joint,0,sizeof(double)*axis_num);
	if(MoveToTargetJoint(&robot,target_joint))
	{
		return;
	}


	string shutdown_flag="n";


	//机械臂关机并失能，在断电之前务必执行此操作
	//请注意要先用手扶住机械臂，否则会因为在关机之后因为重力而下落
//	cout << "Please hold the arm with your hand " ;
//	cout << "and then press enter 'y' to turn off the robot ..." << endl;	
//	cin >> shutdown_flag;

//	if(shutdown_flag == "y")
//	{
		robot.Shutdown();
//	}


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

