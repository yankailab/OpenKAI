/*
 * _PointCloudMerge.cpp
 *
 *  Created on: May 24, 2020
 *      Author: yankai
 */

#include "_PointCloudMerge.h"

#ifdef USE_OPENCV
#ifdef USE_OPEN3D

namespace kai
{

_PointCloudMerge::_PointCloudMerge()
{
	m_pViewer = NULL;

}

_PointCloudMerge::~_PointCloudMerge()
{
}

bool _PointCloudMerge::init(void *pKiss)
{
	IF_F(!_PointCloudBase::init(pKiss));
	Kiss *pK = (Kiss*) pKiss;

	string iName;
	iName = "";
	pK->v("_PointCloudViewer", &iName);
	m_pViewer = (_PointCloudViewer*) (pK->getInst(iName));

	vector<string> vU;
	pK->a("vPCuniv", &vU);
	IF_F(vU.empty());

	for(string u : vU)
	{
		_Universe* pU = (_Universe*) (pK->getInst(u));
		IF_CONT(!pU);

		m_vpU.push_back(pU);
	}
	IF_F(m_vpU.empty());

	m_bOpen = true;

	return true;
}

bool _PointCloudMerge::start(void)
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

void _PointCloudMerge::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		updatePC();
		transform();

		m_pU->updateObj();
		addObj();

		this->autoFPSto();
	}
}

void _PointCloudMerge::updatePC(void)
{
	IF_(check() < 0);

	m_PC.points_.clear();
	m_PC.colors_.clear();
	m_PC.normals_.clear();

	for(_Universe* pU : m_vpU)
	{
		_Object* pO = pU->get(0);
		IF_CONT(!pO);

		vector<Eigen::Vector3d>* pV;

		pV = pO->getPointCloudPoint();
		m_PC.points_.insert(m_PC.points_.end(), pV->begin(), pV->end());

		pV = pO->getPointCloudColor();
		m_PC.colors_.insert(m_PC.colors_.end(), pV->begin(), pV->end());
	}

}

}
#endif
#endif
