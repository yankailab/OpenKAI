/*
 * _RealSensePC.cpp
 *
 *  Created on: May 24, 2020
 *      Author: yankai
 */

#include "_RealSensePC.h"

#ifdef USE_OPENCV
#ifdef USE_OPEN3D
#ifdef USE_REALSENSE

namespace kai
{

_RealSensePC::_RealSensePC()
{
	m_type = pointCloud_realsense;
	m_pRS = NULL;
	m_pViewer = NULL;
}

_RealSensePC::~_RealSensePC()
{
}

bool _RealSensePC::init(void *pKiss)
{
	IF_F(!_PointCloudBase::init(pKiss));
	Kiss *pK = (Kiss*) pKiss;

	string iName;

	iName = "";
	pK->v("_RealSense", &iName);
	m_pRS = (_RealSense*) (pK->getInst(iName));
	IF_Fl(!m_pRS, iName + ": not found");

	iName = "";
	pK->v("_PointCloudViewer", &iName);
	m_pViewer = (_PointCloudViewer*) (pK->getInst(iName));

	return true;
}

bool _RealSensePC::open(void)
{
	NULL_F(m_pRS);

	m_bOpen = m_pRS->isOpened();
	return m_bOpen;
}

void _RealSensePC::close(void)
{
	if (m_threadMode == T_THREAD)
	{
		goSleep();
		while (!bSleeping())
			;
	}

	this->_PointCloudBase::close();
}

bool _RealSensePC::start(void)
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

int _RealSensePC::check(void)
{
	IF__(!open(), -1);
	NULL__(m_pRS, -1);
	IF__(m_pRS->BGR()->m()->empty(), -1);

	return 0;
}

void _RealSensePC::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		updatePC();

		this->autoFPSto();
	}
}

void _RealSensePC::updatePC(void)
{
	IF_(check() < 0);

	m_rsPC.map_to(m_pRS->m_rsColor);
	m_rsPoints = m_rsPC.calculate(m_pRS->m_rsDepth);

	Mat mBGR;
	m_pRS->BGR()->m()->copyTo(mBGR);
	auto rspVertex = m_rsPoints.get_vertices();
	auto rspTexCoord = m_rsPoints.get_texture_coordinates();
	int nP = m_rsPoints.size();

	pthread_mutex_lock(&m_mutex);

	m_PC.points_.clear();
	m_PC.colors_.clear();

	const static float c_b = 1.0 / 255.0;

	for (int i = 0; i < nP; i++)
	{
		rs2::vertex vr = rspVertex[i];
		IF_CONT(vr.z < m_vRz.x);
		IF_CONT(vr.z > m_vRz.y);

		Eigen::Vector3d ve(vr.x, vr.y, vr.z);
		m_PC.points_.push_back(ve);

		rs2::texture_coordinate tc = rspTexCoord[i];
		int tx = constrain<int>(tc.u * mBGR.cols, 0, mBGR.cols - 1);
		int ty = constrain<int>(tc.v * mBGR.rows, 0, mBGR.rows - 1);
		Vec3b vC = mBGR.at<Vec3b>(ty, tx);
		Eigen::Vector3d te(vC[2], vC[1], vC[0]);
		te *= c_b;
		m_PC.colors_.push_back(te);
	}

	transform();

	pthread_mutex_unlock(&m_mutex);

//		auto cIntr = m_pRS->m_cIntrinsics;
//		auto dIntr = m_pRS->m_dIntrinsics;
//		dIntr = cIntr;
//		m_imgD.Prepare(dIntr.width, dIntr.height, 1, 2);
//		m_imgRGB.Prepare(cIntr.width, cIntr.height, 3, 1);
//		memcpy(m_imgD.data_.data(), m_pRS->m_rsDepth.get_data(), dIntr.width * dIntr.height * 2);
//		memcpy(m_imgRGB.data_.data(), m_pRS->m_rsColor.get_data(), cIntr.width * cIntr.height * 3);
//
//		shared_ptr<RGBDImage> imgRGBD = RGBDImage::CreateFromColorAndDepth(m_imgRGB, m_imgD, 1.0/m_pRS->m_dScale, m_pRS->m_vRange.y, false);
//        camera::PinholeCameraIntrinsic camInt(dIntr.width,
//        										dIntr.height,
//												dIntr.fx,
//												dIntr.fy,
//												dIntr.ppx,
//												dIntr.ppy);
//        m_spPC = PointCloud::CreateFromRGBDImage(*imgRGBD, camInt);
//        m_pPC = m_spPC;

}

void _RealSensePC::draw(void)
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
#endif
