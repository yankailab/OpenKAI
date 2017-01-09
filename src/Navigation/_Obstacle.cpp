/*
 * _Obstacle.cpp
 *
 *  Created on: Jan 6, 2017
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
	m_pObs = NULL;
	m_nObs = 128;
	m_iObs = 0;
	m_obsLifetime = USEC_1SEC;

	m_alertDist = 0.0;
	m_detectMinSize = 0.0;
	m_extraBBox = 0.0;
	m_bSlit = false;
	m_slit.init();
	m_bDrawContour = false;
	m_contourBlend = 0.125;

	m_sizeName = 0.5;
	m_sizeDist = 0.5;
	m_colName = Scalar(255,255,0);
	m_colDist = Scalar(0,255,255);
	m_colObs = Scalar(255,255,0);
}

_Obstacle::~_Obstacle()
{
	DEL(m_pObs);
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
	F_INFO(pK->v("extraBBox", &m_extraBBox));
	F_INFO(pK->v("nObs", &m_nObs));
	F_INFO(pK->v("obsLifetime", (int*)&m_obsLifetime));
	F_INFO(pK->v("bDrawContour", &m_bDrawContour));
	F_INFO(pK->v("contourBlend", &m_contourBlend));

	F_INFO(pK->v("sizeName", &m_sizeName));
	F_INFO(pK->v("sizeDist", &m_sizeDist));

	F_INFO(pK->v("nameB", &m_colName[0]));
	F_INFO(pK->v("nameG", &m_colName[1]));
	F_INFO(pK->v("nameR", &m_colName[2]));

	F_INFO(pK->v("distB", &m_colDist[0]));
	F_INFO(pK->v("distG", &m_colDist[1]));
	F_INFO(pK->v("distR", &m_colDist[2]));

	F_INFO(pK->v("obsB", &m_colObs[0]));
	F_INFO(pK->v("obsG", &m_colObs[1]));
	F_INFO(pK->v("obsR", &m_colObs[2]));

	F_INFO(pK->v("bSlit", &m_bSlit));
	if(m_bSlit)
	{
		F_INFO(pK->v("slitL", &m_slit.m_x));
		F_INFO(pK->v("slitR", &m_slit.m_z));
		F_INFO(pK->v("slitT", &m_slit.m_y));
		F_INFO(pK->v("slitB", &m_slit.m_w));
	}

	m_pObs = new OBSTACLE[m_nObs];
	for (int i = 0; i < m_nObs; i++)
	{
		m_pObs[i].m_frameID = 0;
		m_pObs[i].m_dist = -1.0;
		m_pObs[i].m_bPrimaryTarget = false;
	}
	m_iObs = 0;

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
	GpuMat gMat, gMat2;

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
			cv::THRESH_TOZERO);
#else
	gpu::threshold(gMat, gMat2, (1.0 - m_alertDist) * 255.0, 255, cv::THRESH_TOZERO);
#endif

	Mat cMat, cMat2;
	gMat2.download(cMat);
	cMat.copyTo(cMat2);

	vInt4 slitPos;
	if(m_bSlit)
	{
		slitPos.m_x = m_slit.m_x*cMat2.cols;
		slitPos.m_y = m_slit.m_y*cMat2.rows;
		slitPos.m_z = m_slit.m_z*cMat2.cols;
		slitPos.m_w = m_slit.m_w*cMat2.rows;

		Scalar cSlit = Scalar(0,0,0);
		cv::line(cMat2, Point(slitPos.m_x,0), Point(slitPos.m_x,cMat2.rows), cSlit, 2);
		cv::line(cMat2, Point(slitPos.m_z,0), Point(slitPos.m_z,cMat2.rows), cSlit, 2);
		cv::line(cMat2, Point(0,slitPos.m_y), Point(cMat2.cols,slitPos.m_y), cSlit, 2);
		cv::line(cMat2, Point(0,slitPos.m_w), Point(cMat2.cols,slitPos.m_w), cSlit, 2);
	}

	// find contours
	// findContours will modify the contents of the given Mat
	vector<vector<Point> > contours;
	findContours(cMat2, contours, CV_RETR_LIST/*CV_RETR_EXTERNAL*/, CV_CHAIN_APPROX_SIMPLE);

	// Approximate contours to polygons + get bounding rects
	vector<Point> contourPoly;
	double rangeMin, rangeMax;
	m_pStream->getRange(&rangeMin, &rangeMax);
	double range = rangeMax - rangeMin;
	int extraX = cMat.cols*m_extraBBox;
	int extraY = cMat.rows*m_extraBBox;

	for (unsigned int i = 0; i < contours.size(); i++)
	{
		approxPolyDP(Mat(contours[i]), contourPoly, 3, true);
		Rect bb = boundingRect(Mat(contourPoly));
		if (bb.area() < minSize)
			continue;

		if(m_bSlit)
		{
			int margin = 10;
			if(abs(bb.x-slitPos.m_x)<margin &&
					abs(bb.y-slitPos.m_y)<margin &&
					abs(bb.x+bb.width-slitPos.m_z)<margin &&
					abs(bb.y+bb.height-slitPos.m_w)<margin)
				continue;
		}

		OBSTACLE obj;
		obj.m_camSize.m_x = cMat.cols;
		obj.m_camSize.m_y = cMat.rows;
		obj.m_bPrimaryTarget = false;
		if(m_bDrawContour)
			obj.m_contour = contourPoly;

		obj.m_bbox.m_x = bb.x - extraX;
		obj.m_bbox.m_y = bb.y - extraY;
		obj.m_bbox.m_z = bb.x + bb.width + extraX;
		obj.m_bbox.m_w = bb.y + bb.height + extraY;
		if(obj.m_bbox.m_x < 0)obj.m_bbox.m_x = 0;
		if(obj.m_bbox.m_y < 0)obj.m_bbox.m_y = 0;
		if(obj.m_bbox.m_z > cMat.cols)obj.m_bbox.m_z = cMat.cols;
		if(obj.m_bbox.m_w > cMat.rows)obj.m_bbox.m_w = cMat.rows;

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

		obj.m_frameID = get_time_usec();
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
	CHECK_F(pDepth->empty());
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
	m_pObs[m_iObs] = *pNewObj;
	if (++m_iObs >= m_nObs)
		m_iObs = 0;
	return true;
}

int _Obstacle::size(void)
{
	return m_nObs;
}

OBSTACLE* _Obstacle::get(int i, int64_t frameID)
{
	if(frameID - m_pObs[i].m_frameID >= m_obsLifetime)
	{
		return NULL;
	}
	return &m_pObs[i];
}

OBSTACLE* _Obstacle::getByClass(int iClass)
{
	int i;
	OBSTACLE* pObj;

	for (i = 0; i < m_nObs; i++)
	{
		pObj = &m_pObs[i];

		if (pObj->m_iClass == iClass)
			return pObj;
	}

	return NULL;
}

void _Obstacle::info(double* pRangeMin, double* pRangeMax,
		uint8_t* pOrientation)
{
	m_pStream->getRange(pRangeMin, pRangeMax);
	*pRangeMax = *pRangeMin + (*pRangeMax - *pRangeMin)*m_alertDist;
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

	string msg = *this->getName() + " FPS: " + i2str(getFrameRate()) + " AlertDist=" + i2str(aDist);
	pWin->addMsg(&msg);

	Mat bg;
	if (m_bDrawContour)
	{
		bg = Mat::zeros(Size(pMat->cols, pMat->rows), CV_8UC3);
	}

	uint64_t frameID = get_time_usec() - m_obsLifetime;
	for (int i = 0; i < m_nObs; i++)
	{
		OBSTACLE* pObj = get(i, frameID);
		if (!pObj)
			continue;
		if(pObj->m_frameID<=0)
			continue;

		Rect r;
		vInt42rect(&pObj->m_bbox, &r);

		if (pObj->m_iClass>=0)
		{
			putText(*pMat, pObj->m_name,
					Point(r.x + r.width / 2, r.y + r.height / 2),
					FONT_HERSHEY_SIMPLEX, m_sizeName, m_colName, 1);
		}

		putText(*pMat, i2str(pObj->m_dist),
				Point(r.x + r.width / 2, r.y + r.height / 2 + 15),
				FONT_HERSHEY_SIMPLEX, m_sizeDist, m_colDist, 1);

		Scalar colObs = m_colObs;
		int bolObs = 1;
		if(pObj->m_bPrimaryTarget)
		{
			colObs = Scalar(0,0,255);
			bolObs = 2;
		}

		if (m_bDrawContour)
		{
			drawContours(bg, vector<vector<Point> >(1, pObj->m_contour), -1,
					colObs, CV_FILLED, 8);
		}
		else
		{
			rectangle(*pMat, r, colObs, bolObs);
		}
	}

	if (m_bDrawContour)
	{
		cv::addWeighted(*pMat, 1.0, bg, m_contourBlend, 0.0, *pMat);
	}

	return true;
}

}
