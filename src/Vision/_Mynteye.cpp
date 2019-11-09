/*
 * _Mynteye.cpp
 *
 *  Created on: Nov 9, 2019
 *      Author: yankai
 */

#include "_Mynteye.h"

#ifdef USE_MYNTEYE

namespace kai
{

_Mynteye::_Mynteye()
{
	m_type = vision_mynteye;
	m_pDepthWin = NULL;
	m_pTPP = new _ThreadBase();

	m_op.framerate = 30;
	m_op.dev_index = 0;
	m_op.dev_mode = DeviceMode::DEVICE_ALL;
	m_op.color_mode = ColorMode::COLOR_RECTIFIED;
	m_op.depth_mode = DepthMode::DEPTH_GRAY;
	m_op.stream_mode = StreamMode::STREAM_1280x720;
	m_op.state_ae = true;
	m_op.state_awb = true;
	m_op.ir_depth_only = false;
	m_op.ir_intensity = 5;
	m_op.colour_depth_value = 5000;	//[0, 16384]
}

_Mynteye::~_Mynteye()
{
	DEL(m_pTPP);
}

bool _Mynteye::init(void* pKiss)
{
	IF_F(!_DepthVisionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("meFPS", &m_op.framerate);
	pK->v("devIndex", &m_op.dev_index);
	pK->v("devMode", &m_op.dev_mode);
	pK->v("colorMode", &m_op.color_mode);
	pK->v("depthMode", &m_op.depth_mode);
	pK->v("streamMode", &m_op.stream_mode);
	pK->v("bAE", &m_op.state_ae);
	pK->v("bAWB", &m_op.state_awb);
	pK->v("bIRonly", &m_op.ir_depth_only);
	pK->v("IRintensity", &m_op.ir_intensity);
	pK->v("colorDepthValue", &m_op.colour_depth_value);

	return true;
}

bool _Mynteye::open(void)
{
	IF_T(m_bOpen);

	IF_F(!util::select(m_me, &m_devInfo));
	util::print_stream_infos(m_me, m_devInfo.index);
	LOG_I("device: " + i2str(m_devInfo.index) + m_devInfo.name);

	m_op.dev_index = m_devInfo.index;

	// cam.EnableProcessMode(ProcessMode::PROC_IMU_ALL);

	m_me.EnableImageInfo(true);
	m_me.Open(m_op);

	IF_F(!m_me.IsOpened());

	LOG_I("Open device success");
	m_bOpen = true;
	return true;
}

void _Mynteye::close(void)
{
	if (m_threadMode == T_THREAD)
	{
		goSleep();
		while (!bSleeping());
		while (!m_pTPP->bSleeping());
	}

	m_me.Close();
	this->_VisionBase::close();
}

bool _Mynteye::start(void)
{
	IF_F(!this->_ThreadBase::start());

	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		m_bThreadON = false;
		return false;
	}

	retCode = pthread_create(&m_pTPP->m_threadID, 0, getTPP, this);
	if (retCode != 0)
	{
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _Mynteye::update(void)
{
	while (m_bThreadON)
	{
		if (!m_bOpen)
		{
			if (!open())
			{
				LOG_E("Cannot open Mynteye");
				this->sleepTime(USEC_1SEC);
				continue;
			}
		}

		this->autoFPSfrom();

		m_me.WaitForStream();

		auto left_color = m_me.GetStreamData(ImageType::IMAGE_LEFT_COLOR);
		if (left_color.img)
			m_fBGR.copy(left_color.img->To(ImageFormat::COLOR_BGR)->ToMat());

		auto image_depth = m_me.GetStreamData(ImageType::IMAGE_DEPTH);
		if (image_depth.img)
		{
			if (m_op.depth_mode == DepthMode::DEPTH_COLORFUL)
			{
				m_fDepth.copy(image_depth.img->To(ImageFormat::DEPTH_BGR)->ToMat());
			}
			else
			{
				m_fDepth.copy(image_depth.img->ToMat());
			}
		}

		m_pTPP->wakeUp();

		this->autoFPSto();
	}
}

void _Mynteye::updateTPP(void)
{
	while (m_bThreadON)
	{
		m_pTPP->sleepTime(0);

	}
}

void _Mynteye::draw(void)
{
//	m_depthShow = m_fDepth;

	this->_DepthVisionBase::draw();
}

}
#endif
