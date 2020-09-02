/*
 * _PointCloudFile.cpp
 *
 *  Created on: Sept 3, 2020
 *      Author: yankai
 */

#include "_PointCloudFile.h"

#ifdef USE_OPENCV
#ifdef USE_OPEN3D

namespace kai
{

_PointCloudFile::_PointCloudFile()
{
	m_type = pointCloud_realsense;
	m_pViewer = NULL;
}

_PointCloudFile::~_PointCloudFile()
{
}

bool _PointCloudFile::init(void *pKiss)
{
	IF_F(!_PointCloudBase::init(pKiss));
	Kiss *pK = (Kiss*) pKiss;

	pK->v("fName", &m_fName);
	if (!m_fName.empty())
	{
		IF_F(!io::ReadPointCloud(m_fName.c_str(), *m_pPC));
		m_bOpen = true;
	}

	string iName;

	iName = "";
	pK->v("_PointCloudViewer", &iName);
	m_pViewer = (_PointCloudViewer*) (pK->getInst(iName));

	return true;
}

bool _PointCloudFile::open(void)
{
	return m_bOpen;
}

void _PointCloudFile::close(void)
{
	if (m_threadMode == T_THREAD)
	{
		goSleep();
		while (!bSleeping())
			;
	}

	this->_PointCloudBase::close();
}

bool _PointCloudFile::start(void)
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

int _PointCloudFile::check(void)
{
	IF__(!open(), -1);

	return 0;
}

void _PointCloudFile::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		updateFile();

		this->autoFPSto();
	}
}

void _PointCloudFile::updateFile(void)
{
	IF_(check() < 0);

	transform();

}

void _PointCloudFile::draw(void)
{
	this->_PointCloudBase::draw();

	IF_(!m_pViewer);
	m_pViewer->render(m_pPC);
}

}
#endif
#endif
