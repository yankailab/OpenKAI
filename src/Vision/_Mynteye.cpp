/*
 * _Mynteye.cpp
 *
 *  Created on: Nov 9, 2019
 *      Author: yankai
 */

#include "_Mynteye.h"

#ifdef USE_OPENCV
#ifdef USE_MYNTEYE

namespace kai
{

_Mynteye::_Mynteye()
{
	m_type = vision_mynteye;

	m_op.framerate = 30;
	m_op.dev_index = 0;
	m_op.dev_mode = DeviceMode::DEVICE_ALL;
	m_op.color_mode = ColorMode::COLOR_RECTIFIED;
	m_op.depth_mode = DepthMode::DEPTH_RAW;
	m_op.stream_mode = StreamMode::STREAM_1280x720;
	m_op.state_ae = true;
	m_op.state_awb = true;
	m_op.ir_depth_only = false;
	m_op.ir_intensity = 0;
	m_op.colour_depth_value = 5000;	//[0, 16384]

	m_iHistFrom = 1;
	m_dScale = 0.001;

}

_Mynteye::~_Mynteye()
{
}

bool _Mynteye::init(void* pKiss)
{
	IF_F(!_DepthVisionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("meFPS", &m_op.framerate);
	pK->v("devIndex", &m_op.dev_index);
	pK->v("devMode", (int32_t*)&m_op.dev_mode);
	pK->v("colorMode", (int32_t*)&m_op.color_mode);
	pK->v("depthMode", (int32_t*)&m_op.depth_mode);
	pK->v("streamMode", (int32_t*)&m_op.stream_mode);
	pK->v("bAE", &m_op.state_ae);
	pK->v("bAWB", &m_op.state_awb);
	pK->v("bIRdepthOnly", &m_op.ir_depth_only);
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
	this->_VisionBase::close();
	m_me.Close();
}

bool _Mynteye::start(void)
{
    NULL_F(m_pT);
	return m_pT->start(getUpdate, this);
}

void _Mynteye::update(void)
{
	while(m_pT->bRun())
	{
		if (!m_bOpen)
		{
			if (!open())
			{
				LOG_E("Cannot open Mynteye");
				m_pT->sleepTime(SEC_2_USEC);
				continue;
			}
		}

		m_pT->autoFPSfrom();

		m_me.WaitForStream();

		auto imgLeftColor = m_me.GetStreamData(ImageType::IMAGE_LEFT_COLOR);
		if (imgLeftColor.img)
			m_fBGR.copy(imgLeftColor.img->To(ImageFormat::COLOR_BGR)->ToMat());

		auto imgDepth = m_me.GetStreamData(ImageType::IMAGE_DEPTH);
		if (imgDepth.img)
		{
			if (m_op.depth_mode == DepthMode::DEPTH_COLORFUL)
				m_fDepth.copy(imgDepth.img->To(ImageFormat::DEPTH_BGR)->ToMat());
			else
				m_fDepth.copy(imgDepth.img->ToMat());
		}

		m_pT->autoFPSto();
	}
}

void _Mynteye::draw(void* pFrame)
{
	m_fDepthShow.copy(m_fDepth);

	this->_DepthVisionBase::draw(pFrame);
}

}
#endif
#endif
