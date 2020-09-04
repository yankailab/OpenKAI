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
	m_type = pointCloud_realsense;
	m_pViewer = NULL;

	pthread_mutex_init(&m_mutex, NULL);
}

_PointCloudMerge::~_PointCloudMerge()
{
	pthread_mutex_destroy(&m_mutex);
}

bool _PointCloudMerge::init(void *pKiss)
{
	IF_F(!_PointCloudBase::init(pKiss));
	Kiss *pK = (Kiss*) pKiss;

	string iName;
	iName = "";
	pK->v("_PointCloudViewer", &iName);
	m_pViewer = (_PointCloudViewer*) (pK->getInst(iName));

	vector<string> vPC;
	pK->a("vPC", &vPC);
	IF_F(vPC.empty());

	for(string p : vPC)
	{
		iName = "";
		_PointCloudBase* pPC = (_PointCloudBase*) (pK->getInst(iName));
		IF_CONT(!pPC);

		m_vpPC.push_back(pPC);
	}
	IF_F(m_vpPC.empty());

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

int _PointCloudMerge::check(void)
{
	return 0;
}

void _PointCloudMerge::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		updatePC();

		this->autoFPSto();
	}
}

void _PointCloudMerge::updatePC(void)
{
	IF_(check() < 0);

	pthread_mutex_lock(&m_mutex);

	m_PC.points_.clear();
	m_PC.colors_.clear();
	m_PC.normals_.clear();

	for(_PointCloudBase* p : m_vpPC)
	{
		PointCloud pc;
		p->getPointCloud(&pc);

		for (int i = 0; i < pc.points_.size(); i++)
		{
			m_PC.points_.push_back(pc.points_[i]);
			m_PC.points_.push_back(pc.colors_[i]);
		}
	}

	pthread_mutex_unlock(&m_mutex);

	transform();
}

void _PointCloudMerge::draw(void)
{
	this->_PointCloudBase::draw();

	IF_(!m_pViewer);

	pthread_mutex_lock(&m_mutex);
	m_pViewer->render(&m_PC);
	pthread_mutex_unlock(&m_mutex);
}

}
#endif
#endif
