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

	vector<string> vPCB;
	pK->a("vPointCloudBase", &vPCB);
	IF_F(vPCB.empty());

	for(string p : vPCB)
	{
		_PointCloudBase* pPCB = (_PointCloudBase*) (pK->getInst(p));
		IF_CONT(!pPCB);

		m_vpPCB.push_back(pPCB);
	}
	IF_F(m_vpPCB.empty());

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

		m_sPC.update();

		this->autoFPSto();
	}
}

void _PointCloudMerge::updatePC(void)
{
	IF_(check() < 0);

	PointCloud* pPC = m_sPC.next();
	pPC->points_.clear();
	pPC->colors_.clear();
	pPC->normals_.clear();

	for(_PointCloudBase* pPCB : m_vpPCB)
	{
//		IF_CONT(pPCB->m_sPC.prev()->points_.empty());

		vector<Eigen::Vector3d>* pV;
		pV = &pPCB->m_sPC.prev()->points_;
		pPC->points_.insert(pPC->points_.end(), pV->begin(), pV->end());

		pV = &pPCB->m_sPC.prev()->colors_;
		pPC->colors_.insert(pPC->colors_.end(), pV->begin(), pV->end());
	}

	m_pViewer->render(pPC);
}

}
#endif
#endif
