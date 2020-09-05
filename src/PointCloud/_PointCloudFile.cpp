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
	m_type = pointCloud_file;
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
	open();

	string iName;

	iName = "";
	pK->v("_PointCloudViewer", &iName);
	m_pViewer = (_PointCloudViewer*) (pK->getInst(iName));

	return true;
}

bool _PointCloudFile::open(void)
{
	IF_F(m_fName.empty());

	IF_F(!io::ReadPointCloud(m_fName.c_str(), m_PC));
	transform();
	m_bOpen = true;

	return m_bOpen;
}

void _PointCloudFile::close(void)
{
	if (m_threadMode == T_THREAD)
	{
		goSleep();
		while (!bSleeping());
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

}

void _PointCloudFile::transform(void)
{
	Eigen::Matrix4d mT = Eigen::Matrix4d::Identity();
	Eigen::Vector3d vR(m_vR.x, m_vR.y, m_vR.z);
	mT.block(0,0,3,3) = m_PC.GetRotationMatrixFromXYZ(vR);
	mT(0,3) = m_vT.x;
	mT(1,3) = m_vT.y;
	mT(2,3) = m_vT.z;

	pthread_mutex_lock(&m_mutex);
	m_PCprocess = m_PC;
	m_PCprocess.Transform(mT);
	pthread_mutex_unlock(&m_mutex);
}

void _PointCloudFile::getPointCloud(PointCloud* pPC)
{
	NULL_(pPC);

	pthread_mutex_lock(&m_mutex);
	*pPC = m_PCprocess;
	pthread_mutex_unlock(&m_mutex);
}

void _PointCloudFile::draw(void)
{
	this->_PointCloudBase::draw();

	IF_(!m_pViewer);
	pthread_mutex_lock(&m_mutex);
	m_pViewer->render(&m_PCprocess);
	pthread_mutex_unlock(&m_mutex);
}

}
#endif
#endif
