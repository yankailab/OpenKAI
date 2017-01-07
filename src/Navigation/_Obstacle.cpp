/*
 * Object.cpp
 *
 *  Created on: Nov 28, 2015
 *      Author: yankai
 */

#include "_Obstacle.h"

namespace kai
{

_Obstacle::_Obstacle()
{
	m_pStream = NULL;
	m_pIN = NULL;
	m_pFrame = NULL;
	m_pObj = NULL;
	m_nObj = 128;
	m_iObj = 0;
	m_lifetime = USEC_1SEC;

	m_alertDist = 0.0;
	m_detectMinSize = 0.0;
	m_bDrawContour = false;
	m_contourBlend = 0.125;
}

_Obstacle::~_Obstacle()
{
	DEL(m_pObj);
	DEL(m_pFrame);
}

bool _Obstacle::init(void* pKiss)
{
	CHECK_F(!_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	string presetDir = "";
	F_INFO(pK->root()->o("APP")->v("presetDir", &presetDir));
	F_INFO(pK->v("alertDist", &m_alertDist));
	F_INFO(pK->v("detectMinSize", &m_detectMinSize));
	F_INFO(pK->v("nObj", &m_nObj));
	F_INFO(pK->v("lifetime", (int*)&m_lifetime));
	F_INFO(pK->v("bDrawContour", &m_bDrawContour));
	F_INFO(pK->v("contourBlend", &m_contourBlend));

	m_pObj = new OBSTACLE[m_nObj];
	for (int i = 0; i < m_nObj; i++)
	{
		m_pObj[i].m_frameID = 0;
		m_pObj[i].m_dist = -1.0;
	}
	m_iObj = 0;

	m_pFrame = new Frame();

	return true;
}

bool _Obstacle::link(void)
{
	CHECK_F(!this->_ThreadBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	string iName = "";
	F_INFO(pK->v("_Stream", &iName));
	m_pStream = (_StreamBase*) (pK->root()->getChildInstByName(&iName));

	F_INFO(pK->v("_ImageNet", &iName));
	m_pIN = (_ImageNet*) (pK->root()->getChildInstByName(&iName));

	return true;
}

bool _Obstacle::start(void)
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

void _Obstacle::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		detect();

		this->autoFPSto();
	}
}

void _Obstacle::detect(void)
{
	GpuMat gMat;
	GpuMat gMat2;

	NULL_(m_pStream);
	Frame* pDepth = m_pStream->depth();
	NULL_(pDepth);
	CHECK_(pDepth->empty());
	gMat = *(pDepth->getGMat());

	Frame* pFrame = m_pStream->bgr();
	GpuMat gBGR = *pFrame->getGMat();

	//MinSize
	double minSize = m_detectMinSize * gMat.cols * gMat.rows;

#ifndef USE_OPENCV4TEGRA
	cuda::threshold(gMat, gMat2, (1.0 - m_alertDist) * 255.0, 255,
			cv::THRESH_TOZERO); //cv::THRESH_BINARY);
#else
	gpu::threshold(gMat, gMat2, (1.0 - m_alertDist) * 255.0, 255, cv::THRESH_TOZERO);
#endif

	Mat cMat;
	gMat2.download(cMat);

	// Find contours
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	findContours(cMat, contours, hierarchy, CV_RETR_EXTERNAL,
			CV_CHAIN_APPROX_SIMPLE);

	// Approximate contours to polygons + get bounding rects
	vector<Point> contours_poly;
	uint64_t frameID = get_time_usec();
	double rangeMin, rangeMax;
	m_pStream->getRange(&rangeMin, &rangeMax);
	double range = rangeMax - rangeMin;

	for (int i = 0; i < contours.size(); i++)
	{
		approxPolyDP(Mat(contours[i]), contours_poly, 3, true);
		Rect bb = boundingRect(Mat(contours_poly));
		if (bb.area() < minSize)
			continue;

		OBSTACLE obj;
		obj.m_bbox.m_x = bb.x;
		obj.m_bbox.m_y = bb.y;
		obj.m_bbox.m_z = bb.x + bb.width;
		obj.m_bbox.m_w = bb.y + bb.height;
		obj.m_camSize.m_x = cMat.cols;
		obj.m_camSize.m_y = cMat.rows;
		obj.m_contour = contours_poly;
		obj.m_frameID = frameID;

		//calc avr of the region to determine dist
		Mat oMat = Mat(cMat, bb);
		Scalar tot = cv::sum(oMat);
		int area = cv::countNonZero(oMat);
		double cArea = tot[0] / area;
		cArea /= 255.0;
		obj.m_dist = (rangeMax - range * cArea) * 0.1;

		//classify
		obj.m_iClass = -1;
		obj.m_name = "";
		if(m_pIN && !gBGR.empty())
		{
			GpuMat gMat = GpuMat(gBGR,bb);
			m_pFrame->update(&gMat);
			m_pIN->detect(m_pFrame, &obj.m_iClass, &obj.m_name);
		}

		add(&obj);
	}

}

double _Obstacle::dist(Rect* pR)
{
	NULL_F(pR);

	GpuMat gMat;
	GpuMat gMat2;
	GpuMat gHist;
	Mat cHist;

	NULL_F(m_pStream);
	Frame* pDepth = m_pStream->depth();
	NULL_F(pDepth);
	NULL_F(pDepth->empty());
	gMat = *(pDepth->getGMat());
	gMat2 = GpuMat(gMat, *pR);

	int intensity = 0;
	int minPix = pR->area() * m_detectMinSize;

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

	return 1.0;
}

bool _Obstacle::add(OBSTACLE* pNewObj)
{
	NULL_F(pNewObj);
	m_pObj[m_iObj] = *pNewObj;
	if (++m_iObj >= m_nObj)
		m_iObj = 0;
	return true;
}

int _Obstacle::size(void)
{
	return m_nObj;
}

OBSTACLE* _Obstacle::get(int i, int64_t frameID)
{
	if(frameID - m_pObj[i].m_frameID >= m_lifetime)
	{
		return NULL;
	}
	return &m_pObj[i];
}

OBSTACLE* _Obstacle::getByClass(int iClass)
{
	int i;
	OBSTACLE* pObj;

	for (i = 0; i < m_nObj; i++)
	{
		pObj = &m_pObj[i];

		if (pObj->m_iClass == iClass)
			return pObj;
	}

	return NULL;
}

void _Obstacle::info(double* pRangeMin, double* pRangeMax,
		uint8_t* pOrientation)
{
	m_pStream->getRange(pRangeMin, pRangeMax);
	NULL_(pOrientation);
	*pOrientation = m_pStream->getOrientation();
}

bool _Obstacle::draw(void)
{
	CHECK_F(!this->BASE::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Frame* pFrame = pWin->getFrame();
	Mat* pMat = pFrame->getCMat();
	CHECK_F(pMat->empty());

	double rangeMin, rangeMax;
	double aDist = 0.0;
	if(m_pStream)
	{
		m_pStream->getRange(&rangeMin, &rangeMax);
		aDist = (rangeMin + (rangeMax - rangeMin)*m_alertDist)*0.1;
	}

	pWin->tabReset();
	putText(*pMat, *this->getName() + " FPS: " + i2str(getFrameRate()) + " AlertDist=" + i2str(aDist),
			*pWin->getTextPos(), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0),
			1);
	pWin->lineNext();

	Mat bg;
	if (m_bDrawContour)
	{
		bg = Mat::zeros(Size(pMat->cols, pMat->rows), CV_8UC3);
	}

	uint64_t frameID = get_time_usec() - m_dTime;
	for (int i = 0; i < m_nObj; i++)
	{
		OBSTACLE* pObj = get(i, frameID);
		if (!pObj)
			continue;

		Rect r;
		vInt42rect(&pObj->m_bbox, &r);

		if (!pObj->m_name.empty())
		{
			putText(*pMat, pObj->m_name,
					Point(r.x + r.width / 2, r.y + r.height / 2),
					FONT_HERSHEY_SIMPLEX, 0.8, Scalar(0, 255, 0), 1);
		}

		putText(*pMat, i2str(pObj->m_dist),
				Point(r.x + r.width / 2, r.y + r.height / 2 + 15),
				FONT_HERSHEY_SIMPLEX, 0.8, Scalar(0, 255, 255), 1);

		if (m_bDrawContour)
		{
			drawContours(bg, vector<vector<Point> >(1, pObj->m_contour), -1,
					Scalar(0, 255, 0), CV_FILLED, 8);
		}
		else
		{
			rectangle(*pMat, r, Scalar(0, 255, 0), 1);
		}
	}

	if (m_bDrawContour)
	{
		cv::addWeighted(*pMat, 1.0, bg, m_contourBlend, 0.0, *pMat);
	}

	return true;
}

}
