/*
 * ZED.cpp
 *
 *  Created on: Aug 22, 2015
 *      Author: yankai
 */

#include "ZED.h"

namespace kai
{

ZED::ZED()
{
	CamBase();
	m_zedResolution = (int) sl::zed::VGA;
	m_zedMinDist = 1000;
	m_pZed = NULL;
	m_type = CAM_ZED;
	m_zedFPS = DEFAULT_FPS;
}

ZED::~ZED()
{
}

bool ZED::setup(JSON* pJson, string camName)
{
	if (CamBase::setup(pJson, camName) == false)
		return false;

	string presetDir = "";
	string calibFile;

	CHECK_INFO(pJson->getVal("PRESET_DIR", &presetDir));
	CHECK_INFO(pJson->getVal("CAM_" + camName + "_ZED_RESOLUTION",&m_zedResolution));
	CHECK_INFO(pJson->getVal("CAM_" + camName + "_ZED_MIN_DISTANCE",&m_zedMinDist));
	CHECK_INFO(pJson->getVal("CAM_"+camName+"_FPS", &m_zedFPS));

	return true;
}

bool ZED::openCamera(void)
{
	// Initialize ZED color stream in HD and depth in Performance mode
	m_pZed = new sl::zed::Camera((sl::zed::ZEDResolution_mode) m_zedResolution);

	// define a struct of parameters for the initialization
	sl::zed::InitParams zedParams;
	zedParams.mode = sl::zed::MODE::PERFORMANCE;
	zedParams.unit = sl::zed::UNIT::MILLIMETER;
	zedParams.verbose = 1;
	zedParams.device = -1;
	zedParams.minimumDistance = m_zedMinDist;

	sl::zed::ERRCODE err = m_pZed->init(zedParams);
	if (err != sl::zed::SUCCESS)
	{
		LOG(ERROR)<< "ZED Error code: " << sl::zed::errcode2str(err) << std::endl;
		return false;
	}

	m_pZed->setFPS(m_zedFPS);

	// Initialize color image and depth
	m_width = m_pZed->getImageSize().width;
	m_height = m_pZed->getImageSize().height;

	m_centerH = m_width * 0.5;
	m_centerV = m_height * 0.5;

	m_zedMode = sl::zed::STANDARD; //FULL

	m_frame = Mat(m_height, m_width, CV_8UC4, 1);
	m_depthFrame = Mat(m_height, m_width, CV_8UC4, 1);

	return true;
}

GpuMat* ZED::readFrame(void)
{
	// Grab frame and compute depth in FULL sensing mode
	if (m_pZed->grab(m_zedMode))
		return NULL;

	// Retrieve left color image
	sl::zed::Mat gLeft = m_pZed->retrieveImage_gpu(sl::zed::SIDE::LEFT);
	m_Gframe = GpuMat(Size(m_width, m_height), CV_8UC4, gLeft.data);
	cuda::cvtColor(m_Gframe, m_Gframe2, CV_BGRA2BGR);

	// Retrieve depth map
	sl::zed::Mat gDepth = m_pZed->normalizeMeasure_gpu(sl::zed::MEASURE::DEPTH);
	m_Gdepth = GpuMat(Size(m_width, m_height), CV_8UC4, gDepth.data);
	cuda::cvtColor(m_Gdepth, m_Gdepth2, CV_BGRA2GRAY);

	return &m_Gframe2;
}

GpuMat* ZED::getDepthFrame(void)
{
	return &m_Gdepth2;
}

void ZED::release(void)
{
}

} /* namespace kai */
