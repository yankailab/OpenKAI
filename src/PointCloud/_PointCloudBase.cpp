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
	m_pPC = new PointCloud();

	m_vT.init(0);
	m_vR.init(0);
	m_vRz.init(0.0, FLT_MAX);
}

_PointCloudBase::~_PointCloudBase()
{
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
	NULL_(m_pPC);

	Eigen::Matrix4d mT = Eigen::Matrix4d::Identity();
	Eigen::Vector3d vR(m_vR.x, m_vR.y, m_vR.z);
	mT.block(0,0,3,3) = m_pPC->GetRotationMatrixFromXYZ(vR);
	mT(0,3) = m_vT.x;
	mT(1,3) = m_vT.y;
	mT(2,3) = m_vT.z;

	m_pPC->Transform(mT);
}

void _PointCloudBase::draw(void)
{
	this->_ThreadBase::draw();
}

}
#endif
