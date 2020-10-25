/*
 * _PCfile.cpp
 *
 *  Created on: Sept 3, 2020
 *      Author: yankai
 */

#include "_PCfile.h"

#ifdef USE_OPEN3D

namespace kai
{

_PCfile::_PCfile()
{
}

_PCfile::~_PCfile()
{
}

bool _PCfile::init(void *pKiss)
{
	IF_F(!_PCbase::init(pKiss));
	Kiss *pK = (Kiss*) pKiss;

	pK->v("fName", &m_fName);
	open();

	return true;
}

bool _PCfile::open(void)
{
	IF_F(m_fName.empty());

//	io::ReadPointCloudOption ro;
	IF_F(!io::ReadPointCloud(m_fName, *m_sPC.prev()));

	NULL_T(m_pViewer);
	m_pViewer->updateGeometry(m_iV, getPC());

	return true;
}

bool _PCfile::start(void)
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

void _PCfile::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		m_pViewer->updateGeometry(m_iV, getPC());

		this->autoFPSto();
	}
}

void _PCfile::draw(void)
{
	this->_ThreadBase::draw();

//	NULL_(m_pViewer);
//	m_pViewer->updateGeometry(m_iV, getPC());
}

}
#endif
