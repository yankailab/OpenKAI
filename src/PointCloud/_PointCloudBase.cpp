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

}

_PointCloudBase::~_PointCloudBase()
{
}

bool _PointCloudBase::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

//	pK->v("w",&m_w);


	m_bOpen = false;
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

void _PointCloudBase::draw(void)
{
	NULL_(m_pWindow);
	Frame* pFrame = ((Window*)m_pWindow)->getFrame();

	this->_ThreadBase::draw();
}

}
#endif
