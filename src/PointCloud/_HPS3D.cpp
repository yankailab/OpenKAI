/*
 * _HPS3D.cpp
 *
 *  Created on: May 24, 2020
 *      Author: yankai
 */

#include "_HPS3D.h"

#ifdef USE_OPENCV
#ifdef USE_OPEN3D
#ifdef USE_HPS3D

namespace kai
{

_HPS3D::_HPS3D()
{
}

_HPS3D::~_HPS3D()
{
}

bool _HPS3D::init(void *pKiss)
{
	IF_F(!_PCbase::init(pKiss));
	Kiss *pK = (Kiss*) pKiss;

//	pK->v("vRz", &m_vRz);

	string iName;





	char fileName[DEV_NUM][DEV_NAME_SIZE] = {0};
	uint32_t i=0;
	uint32_t n = 0;
	uint32_t a = 0;
	int b  = 0;
	int indx = 0;
	RET_StatusTypeDef ret = RET_OK;

	do
	{
//		HPS3D_SetDebugEnable(false);
//		HPS3D_SetDebugFunc(&User_Printf);
		/*set server IP*/
		ret = HPS3D_SetEthernetServerInfo(&handle[0],"192.168.0.10",12345);
		if(ret != RET_OK)
		{
			printf("HPS3D_SetEthernetServerInfo error ,ret = %d\n",ret);
			break;
		}

		/*Init Device*/
		connect_number = HPS3D_AutoConnectAndInitConfigDevice(handle);
		printf("connect_number = %d\n",connect_number);
		if(connect_number == 0)
		{
			break;
		}

		for(i = 0;i < connect_number;i++)
		{
			My_Observer[i].AsyncEvent = ISubject_Event_DataRecvd;
			My_Observer[i].NotifyEnable = true;
			My_Observer[i].ObserverID = i;
			My_Observer[i].RetPacketType = NULL_PACKET;
		}
		/*Adding asynchronous observers,Only valid in asynchronous or continuous measurement mode*/
		HPS3D_AddObserver(&this->cbRecv, handle, My_Observer);
		HPS3D_SetPointCloudEn(true);
		for(i = 0;i<connect_number;i++)
		{
			HPS3D_SetOpticalEnable(&handle[i],true);
			/*Set to continuous measurement mode*/
			handle[i].RunMode = RUN_CONTINUOUS;
			HPS3D_SetRunMode(&handle[i]);
		}
		a = 1;
	}while(0);


	return true;
}

bool _HPS3D::start(void)
{
	IF_F(!this->_ThreadBase::start());

	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		m_bThreadON = false;
		return false;
	}

	return true;
}

int _HPS3D::check(void)
{

	return this->_PCbase::check();
}

void _HPS3D::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		updateRS();
		m_sPC.update();

		this->autoFPSto();
	}
}

void _HPS3D::updateRS(void)
{
	IF_(check() < 0);

}

}
#endif
#endif
#endif
