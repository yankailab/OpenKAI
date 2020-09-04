/*
 * _PointCloudBase.cpp
 *
 *  Created on: May 24, 2020
 *      Author: yankai
 */

#include "_PointCloudBase.h"

#ifdef USE_OPEN3D

namespace kai
{

_PointCloudBase::_PointCloudBase()
{
	m_bOpen = false;
	m_type = pointCloud_unknown;

	m_vT.init(0);
	m_vR.init(0);
	m_vRz.init(0.0, FLT_MAX);

	pthread_mutex_init(&m_mutex, NULL);

}

_PointCloudBase::~_PointCloudBase()
{
	pthread_mutex_destroy(&m_mutex);
}

bool _PointCloudBase::init(void *pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss *pK = (Kiss*) pKiss;

	pK->v("vT", &m_vT);
	pK->v("vR", &m_vR);
	pK->v("vRz", &m_vRz);

	return true;
}

int _PointCloudBase::size(void)
{
	return -1;
}

POINTCLOUD_TYPE _PointCloudBase::getType(void)
{
	return m_type;
}

bool _PointCloudBase::open(void)
{
	return true;
}

bool _PointCloudBase::isOpened(void)
{
	return m_bOpen;
}

void _PointCloudBase::close(void)
{
	m_bOpen = false;
}

void _PointCloudBase::transform(void)
{
	Eigen::Matrix4d mT = Eigen::Matrix4d::Identity();
	Eigen::Vector3d vR(m_vR.x, m_vR.y, m_vR.z);
	mT.block(0,0,3,3) = m_PC.GetRotationMatrixFromXYZ(vR);
	mT(0,3) = m_vT.x;
	mT(1,3) = m_vT.y;
	mT(2,3) = m_vT.z;

	pthread_mutex_lock(&m_mutex);
	m_PC.Transform(mT);
	pthread_mutex_unlock(&m_mutex);
}

void _PointCloudBase::getPointCloud(PointCloud* pPC)
{
	NULL_(pPC);

	pthread_mutex_lock(&m_mutex);
	*pPC = m_PC;
	pthread_mutex_unlock(&m_mutex);
}

void _PointCloudBase::draw(void)
{
	this->_ThreadBase::draw();
}

}
#endif
