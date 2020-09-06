/*
 * _PointCloudRegistration.cpp
 *
 *  Created on: Sept 6, 2020
 *      Author: yankai
 */

#include "_PointCloudRegistration.h"

#ifdef USE_OPENCV
#ifdef USE_OPEN3D

namespace kai
{

_PointCloudRegistration::_PointCloudRegistration()
{
}

_PointCloudRegistration::~_PointCloudRegistration()
{
}

bool _PointCloudRegistration::init(void *pKiss)
{
	IF_F(!_PointCloudBase::init(pKiss));
	Kiss *pK = (Kiss*) pKiss;

	vector<string> vPC;
	pK->a("vPC", &vPC);
	IF_F(vPC.empty());

	for(string p : vPC)
	{
		_PointCloudBase* pPC = (_PointCloudBase*) (pK->getInst(p));
		IF_CONT(!pPC);

		m_vpPC.push_back(pPC);
	}
	IF_F(m_vpPC.empty());

	return true;
}

bool _PointCloudRegistration::start(void)
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

int _PointCloudRegistration::check(void)
{
	return 0;
}

void _PointCloudRegistration::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		updateRegistration();

		this->autoFPSto();
	}
}

void _PointCloudRegistration::updateRegistration(void)
{
	IF_(check() < 0);

	int nPC = m_vpPC.size();
	for(int i=0; i<nPC; i++)
	{
		_PointCloudBase* pPCsource = m_vpPC[i];
		PointCloud pcSource;
//		pPCsource->getPointCloud(&pcSource);

		int j=i+1;
		if(j>=nPC)j=0;

		_PointCloudBase* pPCtarget = m_vpPC[j];
		PointCloud pcTarget;
//		pPCtarget->getPointCloud(&pcTarget);

		//open3d::registration::RegistrationICP()
	}

}

void _PointCloudRegistration::draw(void)
{
	this->_PointCloudBase::draw();

}

}
#endif
#endif
