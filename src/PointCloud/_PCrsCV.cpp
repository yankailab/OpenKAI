/*
 * _PCrsCV.cpp
 *
 *  Created on: May 24, 2020
 *      Author: yankai
 */

#include "_PCrsCV.h"

#ifdef USE_OPENCV
#ifdef USE_OPEN3D
#ifdef USE_REALSENSE

namespace kai
{

_PCrsCV::_PCrsCV()
{
	m_pRS = NULL;
	m_vRz.init(0.0, FLT_MAX);
}

_PCrsCV::~_PCrsCV()
{
}

bool _PCrsCV::init(void *pKiss)
{
	IF_F(!_PCbase::init(pKiss));
	Kiss *pK = (Kiss*) pKiss;

	pK->v("vRz", &m_vRz);

	string iName;

	iName = "";
	pK->v("_RealSense", &iName);
	m_pRS = (_RealSense*) (pK->getInst(iName));
	IF_Fl(!m_pRS, iName + ": not found");

	return true;
}

bool _PCrsCV::start(void)
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

int _PCrsCV::check(void)
{
	NULL__(m_pRS, -1);
	IF__(!m_pRS->isOpened(), -1);
	IF__(m_pRS->BGR()->m()->empty(), -1);

	return this->_PCbase::check();
}

void _PCrsCV::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		updateRS();
		m_sPC.update();

		if(m_pViewer)
		{
			m_pViewer->updateGeometry(m_iV, getPC());
		}

		this->autoFPSto();
	}
}

void _PCrsCV::updateRS(void)
{
	IF_(check() < 0);

	m_rsPC.map_to(m_pRS->m_rsColor);
	m_rsPoints = m_rsPC.calculate(m_pRS->m_rsDepth);

	Mat mBGR;
	m_pRS->BGR()->m()->copyTo(mBGR);
	auto rspVertex = m_rsPoints.get_vertices();
	auto rspTexCoord = m_rsPoints.get_texture_coordinates();
	int nP = m_rsPoints.size();

	PointCloud* pPC = m_sPC.next();
	pPC->points_.clear();
	pPC->colors_.clear();
	pPC->normals_.clear();

	const static float c_b = 1.0 / 255.0;

	for (int i = 0; i < nP; i++)
	{
		rs2::vertex vr = rspVertex[i];
		IF_CONT(vr.z < m_vRz.x);
		IF_CONT(vr.z > m_vRz.y);

		Eigen::Vector3d ve(vr.x, vr.y, vr.z);
		pPC->points_.push_back(ve);

		rs2::texture_coordinate tc = rspTexCoord[i];
		int tx = constrain<int>(tc.u * mBGR.cols, 0, mBGR.cols - 1);
		int ty = constrain<int>(tc.v * mBGR.rows, 0, mBGR.rows - 1);
		Vec3b vC = mBGR.at<Vec3b>(ty, tx);
		Eigen::Vector3d te(vC[2], vC[1], vC[0]);
		te *= c_b;
		pPC->colors_.push_back(te);
	}


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

}
#endif
#endif
#endif
