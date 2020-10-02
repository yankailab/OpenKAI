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
	m_pPCB = NULL;
	m_pViewer = NULL;
}

_PointCloudBase::~_PointCloudBase()
{
}

bool _PointCloudBase::init(void *pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss *pK = (Kiss*) pKiss;

	int nPCreserve = 0;
	pK->v("nPCreserve", &nPCreserve);
	if(nPCreserve > 0)
	{
		m_sPC.prev()->points_.reserve(nPCreserve);
		m_sPC.prev()->colors_.reserve(nPCreserve);
		m_sPC.next()->points_.reserve(nPCreserve);
		m_sPC.next()->colors_.reserve(nPCreserve);
	}

	string iName;

	iName = "";
	pK->v("_PointCloudBase", &iName);
	m_pPCB = (_PointCloudBase*) (pK->getInst(iName));

	iName = "";
	pK->v("_PointCloudViewer", &iName);
	m_pViewer = (_PCviewer*) (pK->getInst(iName));

	return true;
}

int _PointCloudBase::check(void)
{
	return 0;
}

PointCloud* _PointCloudBase::getPC(void)
{
	return m_sPC.prev();
}

int _PointCloudBase::size(void)
{
	return m_sPC.prev()->points_.size();
}

void _PointCloudBase::draw(void)
{
	this->_ThreadBase::draw();

	NULL_(m_pViewer);
//	m_pViewer->render(m_sPC.prev());
}

}
#endif
