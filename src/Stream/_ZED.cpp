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
	m_bZedFlip = false;
	m_zedConfidence = 100;
	m_angleH = 66.7;
	m_angleV = 67.1;
	m_bNormalize = true;
	m_pNorm = NULL;
	m_pNormWin = NULL;
}

_ZED::~_ZED()
{
	this->_StreamBase::complete();
	DEL(m_pNorm);
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
	F_INFO(pK->v("bZedFlip", &m_bZedFlip));
	F_INFO(pK->v("zedConfidence", &m_zedConfidence));
	F_INFO(pK->v("bNormalize", &m_bNormalize));

	m_pDepth = new Frame();

	if(m_bNormalize)
		m_pNorm = new Frame();

	return true;
}

bool _ZED::link(void)
{
	CHECK_F(!this->_StreamBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	string iName = "";
	F_INFO(pK->v("depthWindow", &iName));
	m_pDepthWin = (Window*) (pK->root()->getChildInstByName(&iName));

	F_INFO(pK->v("normWindow", &iName));
	m_pNormWin = (Window*) (pK->root()->getChildInstByName(&iName));

	return true;
}

bool _ZED::open(void)
{
	// Initialize ZED color stream in HD and depth in Performance mode
	m_pZed = new sl::zed::Camera((sl::zed::ZEDResolution_mode) m_zedResolution);

	// define a struct of parameters for the initialization
	sl::zed::InitParams zedParams;
	zedParams.mode = (sl::zed::MODE) m_zedQuality;
	zedParams.unit = sl::zed::UNIT::METER;
	zedParams.verbose = 1;
	zedParams.device = -1;
	zedParams.minimumDistance = m_zedMinDist;
	zedParams.vflip = m_bZedFlip;

	sl::zed::ERRCODE err = m_pZed->init(zedParams);
	if (err != sl::zed::SUCCESS)
	{
		LOG(ERROR)<< "ZED Error code: " << sl::zed::errcode2str(err) << std::endl;
		return false;
	}

	m_pZed->setFPS(m_zedFPS);
	m_pZed->setConfidenceThreshold(m_zedConfidence);
	m_pZed->setDepthClampValue(m_zedMaxDist);
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

		GpuMat gImg;
		GpuMat gImg2;
		GpuMat gDepth;
		GpuMat gDepth2;
		GpuMat gNorm;
		GpuMat gNorm2;

		GpuMat* pSrc;
		GpuMat* pDest;
		GpuMat* pSrcD;
		GpuMat* pDestD;
		GpuMat* pSrcN;
		GpuMat* pDestN;
		GpuMat* pTmp;

		// Grab frame and compute depth in FULL sensing mode
		if (!m_pZed->grab(m_zedMode))
		{
			sl::zed::Mat zLeft = m_pZed->retrieveImage_gpu(sl::zed::SIDE::LEFT);
			gImg = GpuMat(Size(zLeft.width, zLeft.height), CV_8UC4, zLeft.data);

			sl::zed::Mat zDepth = m_pZed->retrieveMeasure_gpu(sl::zed::MEASURE::DEPTH);
			gDepth = GpuMat(Size(zDepth.width, zDepth.height), CV_32F, zDepth.data);

			// Retrieve normalized map
			sl::zed::Mat zNormalize;
			if(m_bNormalize)
			{
				zNormalize = m_pZed->normalizeMeasure_gpu(sl::zed::MEASURE::DEPTH,m_zedMinDist,m_zedMaxDist);
				gNorm = GpuMat(Size(zNormalize.width, zNormalize.height), CV_8UC4, zNormalize.data);
			}


#ifndef USE_OPENCV4TEGRA
			cuda::cvtColor(gImg, gImg2, CV_BGRA2BGR);
			if(m_bNormalize)
			{
				cuda::cvtColor(gNorm, gNorm2, CV_BGRA2GRAY);
			}
#else
			gpu::cvtColor(gImg, gImg2, CV_BGRA2BGR);
			if(m_bNormalize)
			{
				gpu::cvtColor(gNorm, gNorm2, CV_BGRA2GRAY);
			}
#endif
			pSrc = &gImg2;
			pDest = &gImg;
			pSrcD = &gDepth;
			pDestD = &gDepth2;
			pSrcN = &gNorm2;
			pDestN = &gNorm;

			if(m_bFlip)
			{
#ifndef USE_OPENCV4TEGRA
				cuda::flip(*pSrc,*pDest,-1);
				cuda::flip(*pSrcD,*pDestD,-1);
				if(m_bNormalize)
				{
					cuda::flip(*pSrcN,*pDestN,-1);
				}
#else
				gpu::flip(*pSrc,*pDest,-1);
				gpu::flip(*pSrcD,*pDestD,-1);
				if(m_bNormalize)
				{
					gpu::flip(*pSrcN,*pDestN,-1);
				}
#endif
				SWAP(pSrc, pDest, pTmp);
				SWAP(pSrcD, pDestD, pTmp);
				SWAP(pSrcN, pDestN, pTmp);
			}

			m_pBGR->update(pSrc);
			if (m_pGray)
				m_pGray->getGrayOf(m_pBGR);
			if (m_pHSV)
				m_pHSV->getHSVOf(m_pBGR);

			m_pDepth->update(pSrcD);

			if(m_bNormalize)
			{
				m_pNorm->update(pSrcN);
			}
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

Frame* _ZED::norm(void)
{
	return m_pNorm;
}

double _ZED::dist(Rect* pR)
{
	NULL_F(pR);

	GpuMat gMat;
	GpuMat gMat2;
	GpuMat gHist;
	Mat cHist;

	NULL_F(m_pDepth);
	CHECK_F(m_pDepth->empty());
	gMat = *(m_pDepth->getGMat());
	gMat2 = GpuMat(gMat, *pR);

	int intensity = 0;
	int minPix = pR->area() * 0.5;

#ifndef USE_OPENCV4TEGRA
	cuda::calcHist(gMat2, gHist);
	gHist.download(cHist);

	for (int i = cHist.cols - 1; i > 0; i--)
	{
		intensity += cHist.at<int>(0, i);
		if (intensity > minPix)
		{
			return (255.0f - i) / 255.0f;
		}
	}
#else
	int channels[] =
	{	0};
	int bin_num = 256;
	int bin_nums[] =
	{	bin_num};
	float range[] =
	{	0, 256};
	const float *ranges[] =
	{	range};
	Mat cMat;
	gMat2.download(cMat);
	cv::calcHist(&cMat, 1, channels, cv::Mat(), cHist, 1, bin_nums, ranges);

	for (int i = cHist.rows-1; i > 0; i--)
	{
		intensity += cHist.at<int>(i, 0);
		if(intensity > minPix)
		{
			return (255.0f - i)/255.0f;
		}
	}
#endif

	return -1.0;
}

bool _ZED::draw(void)
{
	Window* pWin;
	Frame* pFrame;

	if(this->BASE::draw())
	{
		CHECK_F(m_pBGR->empty());
		pWin = (Window*) this->m_pWindow;
		pFrame = pWin->getFrame();
		pFrame->update(m_pBGR);
		this->_StreamBase::draw();
	}

	if(m_pDepthWin)
	{
		pFrame = m_pDepthWin->getFrame();
		if(pFrame && !m_pDepth->empty())
		{
			GpuMat gD;
#ifndef USE_OPENCV4TEGRA
			cuda::multiply(*m_pDepth->getGMat(), Scalar(1.0/m_zedMaxDist), gD);
#else
			gpu::multiply(*m_pDepth->getGMat(), Scalar(1.0/m_zedMaxDist), gD);
#endif
			pFrame->update(&gD);
		}
	}

	if(m_pNormWin && m_pNorm)
	{
		pFrame = m_pNormWin->getFrame();
		if(pFrame && !m_pNorm->empty())
		{
			pFrame->update(m_pNorm);
		}
	}

	return true;
}

}

#endif
