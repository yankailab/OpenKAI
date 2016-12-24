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
	m_zedMinDist = 1000;
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
	F_INFO(pK->v("zedMinDist", &m_zedMinDist));
	F_INFO(pK->v("zedFPS", &m_zedFPS));
	F_INFO(pK->v("zedQuality", &m_zedQuality));

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
	zedParams.mode = (sl::zed::MODE)m_zedQuality;//sl::zed::MODE::PERFORMANCE;
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

	m_zedMode = sl::zed::STANDARD;

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
					sl::zed::MEASURE::DEPTH);

			m_Gframe = GpuMat(Size(m_width, m_height), CV_8UC4, gLeft.data);
			m_Gdepth = GpuMat(Size(m_width, m_height), CV_8UC4, gDepth.data);

#ifndef USE_OPENCV4TEGRA
			cuda::cvtColor(m_Gframe, m_Gframe2, CV_BGRA2BGR);
			cuda::cvtColor(m_Gdepth, m_Gdepth2, CV_BGRA2GRAY);
#else
			gpu::cvtColor(m_Gframe, m_Gframe2, CV_BGRA2BGR);
			gpu::cvtColor(m_Gdepth, m_Gdepth2, CV_BGRA2GRAY);
#endif

			m_pBGR->update(&m_Gframe2);
			if (m_pGray)
				m_pGray->getGrayOf(m_pBGR);
			if (m_pHSV)
				m_pHSV->getHSVOf(m_pBGR);

			m_pDepth->update(&m_Gdepth2);
		}

		this->autoFPSto();
	}
}

bool _ZED::distNearest(vDouble4* pRect, double* pDist, double* pSize)
{
	NULL_F(pRect);
	NULL_F(pDist);
	NULL_F(pSize);
	CHECK_F(pRect->area() <= 0);

	GpuMat gMat;
	GpuMat gMat2;
	Mat histMat;

	NULL_F(m_pDepth);
	gMat = *(m_pDepth->getGMat());
	CHECK_F(gMat.empty());

	//MinSize
	int minSize = 0;
	if (*pSize > 0)
		minSize = *pSize * gMat.cols * gMat.rows;

	//Region
	Rect r;
	r.x = pRect->m_x * ((double) gMat.cols);
	r.y = pRect->m_y * ((double) gMat.rows);
	r.width = pRect->m_z * ((double) gMat.cols) - r.x;
	r.height = pRect->m_w * ((double) gMat.rows) - r.y;
	gMat2 = GpuMat(gMat, r);

#ifndef USE_OPENCV4TEGRA
	cuda::calcHist(gMat2, gMat);
	gMat.download(histMat);
	for (int i = histMat.cols - 1; i > 0; i--)
	{
		int intensity = histMat.at<int>(0, i);
		if (intensity > minSize)
		{
			*pDist = (255.0f - i) / 255.0f;
			*pSize = intensity;
			return true;
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
	cv::calcHist(&cMat, 1, channels, cv::Mat(), histMat, 1, bin_nums, ranges);

	for (int i = histMat.rows-1; i > 0; i--)
	{
		int intensity = histMat.at<int>(i, 0);
		if(intensity > minSize)
		{
			*pDist = (255.0f - i)/255.0f;
			*pSize = intensity;
			return true;
		}
	}
#endif

	*pDist = -1.0;
	*pSize = 0;
	return true;
}

int _ZED::findObjects(vDouble4* pRect, Object* pResult, double dist, double minSize)
{
	NULL_F(pRect);
	NULL_F(pResult);
	CHECK_F(pRect->area() <= 0);

	GpuMat gMat;
	GpuMat gMat2;

	NULL_F(m_pDepth);
	gMat = *(m_pDepth->getGMat());
	CHECK_F(gMat.empty());

	//MinSize
	minSize *= gMat.cols * gMat.rows;

	//Region
	Rect r;
	r.x = pRect->m_x * ((double) gMat.cols);
	r.y = pRect->m_y * ((double) gMat.rows);
	r.width = pRect->m_z * ((double) gMat.cols) - r.x;
	r.height = pRect->m_w * ((double) gMat.rows) - r.y;
	gMat2 = GpuMat(gMat, r);

#ifndef USE_OPENCV4TEGRA
	cuda::threshold(gMat2, gMat, (1.0-dist)*255.0, 255, cv::THRESH_BINARY);
#else
	gpu::threshold(gMat2, gMat, (1.0-dist)*255.0, 255, cv::THRESH_BINARY);
#endif

	Mat cMat;
	gMat.download(cMat);

	// Find contours
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	findContours(cMat, contours, hierarchy, CV_RETR_EXTERNAL,
			CV_CHAIN_APPROX_SIMPLE);

	// Approximate contours to polygons + get bounding rects
	//vector<vector<Point> > contours_poly(contours.size());
	vector<Point> contours_poly;
	for (int i = 0; i < contours.size(); i++)
	{
//		approxPolyDP(Mat(contours[i]), contours_poly[i], 3, true);
		approxPolyDP(Mat(contours[i]), contours_poly, 3, true);
		Rect bb = boundingRect(Mat(contours_poly));
		if (bb.area() < minSize)continue;

		OBJECT obj;
		obj.m_bbox.m_x = bb.x;
		obj.m_bbox.m_y = bb.y;
		obj.m_bbox.m_z = bb.x + bb.width;
		obj.m_bbox.m_w = bb.y + bb.height;
		obj.m_camSize.m_x = cMat.cols;
		obj.m_camSize.m_y = cMat.rows;
		obj.m_dist = dist;

		pResult->add(&obj);
	}

	return pResult->size();
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
