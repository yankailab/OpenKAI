/*
 * ZED.cpp
 *
 *  Created on: Aug 22, 2015
 *      Author: yankai
 */

#include "_ZED.h"

#ifdef USE_ZED

namespace kai
{

_ZED::_ZED()
{
	_StreamBase();

	m_type = zed;
	m_zedResolution = (int) sl::zed::VGA;
	m_zedMinDist = 600;
	m_zedMaxDist = 15000;
	m_pZed = NULL;
	m_zedFPS = DEFAULT_FPS;
	m_zedMode = sl::zed::STANDARD;
	m_zedQuality = sl::zed::PERFORMANCE;
	m_pDepthWin = NULL;
}

_ZED::~_ZED()
{
	this->_StreamBase::complete();
}

bool _ZED::init(void* pKiss)
{
	CHECK_F(!_StreamBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	string presetDir = "";
	string calibFile;

	F_INFO(pK->root()->o("APP")->v("presetDir", &presetDir));
	F_INFO(pK->v("zedResolution", &m_zedResolution));
	F_INFO(pK->v("zedFPS", &m_zedFPS));
	F_INFO(pK->v("zedQuality", &m_zedQuality));
	F_INFO(pK->v("zedMinDist", &m_zedMinDist));
	F_INFO(pK->v("zedMaxDist", &m_zedMaxDist));

	m_pDepth = new Frame();
	return true;
}

bool _ZED::link(void)
{
	CHECK_F(!this->_StreamBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	string iName = "";
	F_INFO(pK->v("depthWindow", &iName));
	m_pDepthWin = (Window*) (pK->root()->getChildInstByName(&iName));

	return true;
}

bool _ZED::open(void)
{
	// Initialize ZED color stream in HD and depth in Performance mode
	m_pZed = new sl::zed::Camera((sl::zed::ZEDResolution_mode) m_zedResolution);

	// define a struct of parameters for the initialization
	sl::zed::InitParams zedParams;
	zedParams.mode = (sl::zed::MODE) m_zedQuality; //sl::zed::MODE::PERFORMANCE;
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
	m_zedMode = sl::zed::STANDARD;

	// Initialize color image and depth
	m_width = m_pZed->getImageSize().width;
	m_height = m_pZed->getImageSize().height;
	m_centerH = m_width * 0.5;
	m_centerV = m_height * 0.5;

	m_bOpen = true;
	return true;
}

bool _ZED::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _ZED::update(void)
{
	while (m_bThreadON)
	{
		if (!m_bOpen)
		{
			if (!open())
			{
				LOG_E("Cannot open ZED");
				this->sleepThread(USEC_1SEC);
				continue;
			}
		}

		this->autoFPSfrom();

		// Grab frame and compute depth in FULL sensing mode
		if (!m_pZed->grab(m_zedMode))
		{
			// Retrieve left color image
			sl::zed::Mat gLeft = m_pZed->retrieveImage_gpu(sl::zed::SIDE::LEFT);
			// Retrieve depth map
			sl::zed::Mat gDepth = m_pZed->normalizeMeasure_gpu(
					sl::zed::MEASURE::DEPTH,
					m_zedMinDist,
					m_zedMaxDist);

			m_Gmat = GpuMat(Size(m_width, m_height), CV_8UC4, gLeft.data);
			m_Gdepth = GpuMat(Size(m_width, m_height), CV_8UC4, gDepth.data);

#ifndef USE_OPENCV4TEGRA
			cuda::cvtColor(m_Gmat, m_Gmat2, CV_BGRA2BGR);
			cuda::cvtColor(m_Gdepth, m_Gdepth2, CV_BGRA2GRAY);
#else
			gpu::cvtColor(m_Gmat, m_Gmat2, CV_BGRA2BGR);
			gpu::cvtColor(m_Gdepth, m_Gdepth2, CV_BGRA2GRAY);
#endif

			m_pBGR->update(&m_Gmat2);
			if (m_pGray)
				m_pGray->getGrayOf(m_pBGR);
			if (m_pHSV)
				m_pHSV->getHSVOf(m_pBGR);

			m_pDepth->update(&m_Gdepth2);
		}

		this->autoFPSto();
	}
}

void _ZED::getRange(double* pMin, double* pMax)
{
	NULL_(pMin);
	NULL_(pMax);

	*pMin = m_zedMinDist;
	*pMax = m_zedMaxDist;
}

bool _ZED::draw(void)
{
	CHECK_F(!this->BASE::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Frame* pFrame = pWin->getFrame();
	pFrame->update(m_pBGR);
	this->_StreamBase::draw();

	NULL_T(m_pDepthWin);
	pFrame = m_pDepthWin->getFrame();
	pFrame->update(m_pDepth);

	return true;
}

}

#endif
