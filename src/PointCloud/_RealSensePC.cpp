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

bool _RealSensePC::init(void* pKiss)
{
	IF_F(!_PointCloudBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	string iName;

	iName = "";
	pK->v("_RealSense", &iName);
	m_pRS = (_RealSense*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pRS, iName + ": not found");

	iName = "";
	pK->v("_PointCloudViewer", &iName);
	m_pViewer = (_PointCloudViewer*) (pK->root()->getChildInst(iName));

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
		while (!bSleeping());
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

void _RealSensePC::update(void)
{
	if(m_pViewer)
	{
		while(!m_pViewer->isOpened())
			this->sleepTime(USEC_1SEC);
	}

	while (m_bThreadON)
	{
		if (!m_bOpen)
		{
			if (!open())
			{
				LOG_E("Cannot open RealSensePC");
				this->sleepTime(USEC_1SEC);
				continue;
			}
		}

		this->autoFPSfrom();

        m_rsPC.map_to(m_pRS->m_rsColor);
        m_rsPoints = m_rsPC.calculate(m_pRS->m_rsDepth);


        auto cIntr = m_pRS->m_cIntrinsics;
        auto dIntr = m_pRS->m_dIntrinsics;

		auto spImgD = make_shared<Image>();
		spImgD->Prepare(dIntr.width, dIntr.height, 1, 2);
		memcpy(spImgD->data_.data(), m_pRS->m_rsDepth.get_data(), dIntr.width * dIntr.height * 2);

		auto spImgRGB = make_shared<Image>();
		spImgRGB->Prepare(cIntr.width, cIntr.height, 3, 1);
		memcpy(spImgRGB->data_.data(), m_pRS->m_rsColor.get_data(), cIntr.width * cIntr.height * 3);

		shared_ptr<RGBDImage> imgRGBD;// = shared_ptr<RGBDImage>(new RGBDImage);
		imgRGBD = RGBDImage::CreateFromColorAndDepth(*spImgRGB, *spImgD);

        camera::PinholeCameraIntrinsic camInt(dIntr.width,
        										dIntr.height,
												dIntr.fx,
												dIntr.fy,
												dIntr.ppx,
												dIntr.ppy);

        m_spPC = PointCloud::CreateFromRGBDImage(*imgRGBD, camInt);
        *m_pViewer->m_spPC = *m_spPC;

		this->autoFPSto();
	}
}

void _RealSensePC::draw(void)
{
	this->_PointCloudBase::draw();

}

}
#endif
#endif
#endif
