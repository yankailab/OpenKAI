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
	m_pObj = NULL;
	m_bDetectObject = true;
	m_alertDist = 0.0;
	m_detectMinSize = 0.0;
	m_nObj = 128;
	m_oLifetime = 100000;
	m_bDrawContour = false;
	m_contourBlend = 0.125;
	m_distMinSize = 0.0;
}

_ZED::~_ZED()
{
	this->_StreamBase::complete();
	DEL(m_pObj);
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
	F_INFO(pK->v("bDetectObject", &m_bDetectObject));
	F_INFO(pK->v("alertDist", &m_alertDist));
	F_INFO(pK->v("detectMinSize", &m_detectMinSize));
	F_INFO(pK->v("zedMinDist", &m_zedMinDist));
	F_INFO(pK->v("zedMaxDist", &m_zedMaxDist));
	F_INFO(pK->v("nObj", &m_nObj));
	F_INFO(pK->v("objLifetime", &m_oLifetime));
	F_INFO(pK->v("bDrawContour", &m_bDrawContour));
	F_INFO(pK->v("contourBlend", &m_contourBlend));
	F_INFO(pK->v("distMinSize", &m_distMinSize));

	m_pDepth = new Frame();
	m_pObj = new Object(m_nObj,m_oLifetime);
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

			if(m_bDetectObject)
				detectObject();
		}

		this->autoFPSto();
	}
}

void _ZED::detectObject(void)
{
	GpuMat gMat;
	GpuMat gMat2;

	NULL_(m_pDepth);
	gMat = *(m_pDepth->getGMat());
	CHECK_(gMat.empty());

	//MinSize
	double minSize = m_detectMinSize * gMat.cols * gMat.rows;

#ifndef USE_OPENCV4TEGRA
	cuda::threshold(gMat, gMat2, (1.0 - m_alertDist) * 255.0, 255, cv::THRESH_TOZERO);//cv::THRESH_BINARY);
#else
	gpu::threshold(gMat, gMat2, (1.0-dist)*255.0, 255, cv::THRESH_BINARY);
#endif

	Mat cMat;
	gMat2.download(cMat);

	// Find contours
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	findContours(cMat, contours, hierarchy, CV_RETR_EXTERNAL,
			CV_CHAIN_APPROX_SIMPLE);

	uint64_t frameID = get_time_usec();

	// Approximate contours to polygons + get bounding rects
	vector<Point> contours_poly;
	for (int i = 0; i < contours.size(); i++)
	{
		approxPolyDP(Mat(contours[i]), contours_poly, 3, true);
		Rect bb = boundingRect(Mat(contours_poly));
		if (bb.area() < minSize)
			continue;

		OBJECT obj;
		obj.m_bbox.m_x = bb.x;
		obj.m_bbox.m_y = bb.y;
		obj.m_bbox.m_z = bb.x + bb.width;
		obj.m_bbox.m_w = bb.y + bb.height;
		obj.m_camSize.m_x = cMat.cols;
		obj.m_camSize.m_y = cMat.rows;
		obj.m_contour = contours_poly;
		obj.m_frameID = frameID;

		//calc avr of the region to determine dist
		Mat oGMat = Mat(cMat,bb);
		Scalar tot = cv::sum(oGMat);
		obj.m_dist =  m_zedMinDist+(m_zedMaxDist-m_zedMinDist)*((tot[0]/cv::contourArea(contours_poly)) / 255.0);
		obj.m_dist*=0.1;

		m_pObj->add(&obj);
	}

}

Object* _ZED::getObject(void)
{
	return m_pObj;
}

double _ZED::dist(Rect* pR)
{
	NULL_F(pR);

	GpuMat gMat;
	GpuMat gMat2;
	GpuMat gHist;
	Mat cHist;

	NULL_F(m_pDepth);
	gMat = *(m_pDepth->getGMat());
	CHECK_F(gMat.empty());
	gMat2 = GpuMat(gMat, *pR);

	int intensity = 0;
	int minPix = pR->area()*m_distMinSize;

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
	int channels[] = {0};
	int bin_num = 256;
	int bin_nums[] = {bin_num};
	float range[] =	{0, 256};
	const float *ranges[] = {range};
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

	return 1.0;
}

bool _ZED::draw(void)
{
	CHECK_F(!this->BASE::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Frame* pFrame = pWin->getFrame();

	pFrame->update(m_pBGR);
	this->_StreamBase::draw();
	Mat* pMat = pFrame->getCMat();
	CHECK_F(pMat->empty());

	Mat bg;
	if(m_bDrawContour)
	{
		bg = Mat::zeros(Size(pMat->cols,pMat->rows), CV_8UC3);
	}

	uint64_t frameID = get_time_usec()-m_dTime;
	for (int i=0; i<m_pObj->size(); i++)
	{
		OBJECT* pObj = m_pObj->get(i,frameID);
		if(!pObj)continue;

		Rect r;
		vInt42rect(&pObj->m_bbox, &r);

		if(!pObj->m_name.empty())
		{
			putText(*pMat, pObj->m_name,
					Point(r.x + r.width / 2, r.y + r.height / 2),
					FONT_HERSHEY_SIMPLEX, 0.8, Scalar(0, 255, 0), 1);
		}

		putText(*pMat, i2str(pObj->m_dist),
				Point(r.x + r.width / 2, r.y + r.height / 2 + 15),
				FONT_HERSHEY_SIMPLEX, 0.8, Scalar(0, 255, 255), 1);

		if(m_bDrawContour)
		{
			drawContours(bg, vector<vector<Point> >(1,pObj->m_contour), -1, Scalar(0, 255, 0), CV_FILLED, 8);
		}
		else
		{
			rectangle(*pMat, r, Scalar(0, 255, 0), 1);
		}
	}

	if(m_bDrawContour)
	{
		cv::addWeighted( *pMat, 1.0, bg, m_contourBlend, 0.0, *pMat);
	}

	NULL_T(m_pDepthWin);
	pFrame = m_pDepthWin->getFrame();
	pFrame->update(m_pDepth);

	return true;
}

}

#endif
