/*
 * _Depth.cpp
 *
 *  Created on: Aug 17, 2015
 *      Author: yankai
 */

/*
#include "_Depth.h"

namespace kai
{
_Depth::_Depth()
{
	_ThreadBase();

	m_pStream = NULL;
	m_pUniverse = NULL;
	m_pThrFrame = NULL;
	m_minArea = 0;
	m_threshold = 0;
	m_maxValue = 255;
}

_Depth::~_Depth()
{
}

bool _Depth::init(void* pKiss)
{
	CHECK_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	F_INFO(pK->v("areaMin", &m_minArea));
	F_INFO(pK->v("threshold", &m_threshold));
	F_INFO(pK->v("maxVal", &m_maxValue));

	m_pThrFrame = new Frame();

	return true;
}

bool _Depth::link(void)
{
	CHECK_F(!this->_ThreadBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	//link instance
	string iName = "";
	F_ERROR_F(pK->v("_Stream", &iName));
	m_pStream = (_StreamBase*) (pK->root()->getChildInstByName(&iName));
	F_ERROR_F(pK->v("_Universe", &iName));
	m_pUniverse = (_Universe*) (pK->root()->getChildInstByName(&iName));

	return true;
}

bool _Depth::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		LOG_E(retCode);
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _Depth::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		detect();

		this->autoFPSto();
	}

}

void _Depth::detect(void)
{
	NULL_(m_pUniverse);
	NULL_(m_pStream);

	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	GpuMat gMat;
	GpuMat gMat2;
	OBJECT obj;

	gMat = *(m_pStream->depth()->getGMat());
	CHECK_(gMat.empty());

	//TODO: general multiple layers
//		cuda::divide(gMat,Scalar(50),gMat2);
//		cuda::multiply(gMat2,Scalar(50),gMat);

#ifndef USE_OPENCV4TEGRA
	cuda::threshold(gMat, gMat2, m_threshold, m_maxValue, cv::THRESH_BINARY);
#else
	gpu::threshold(gMat, gMat2, m_threshold, m_maxValue, cv::THRESH_TOZERO);
#endif

	gMat2.download(m_Mat);
	m_pThrFrame->update(&gMat2);

	// Find contours
	findContours(m_Mat, contours, hierarchy, CV_RETR_EXTERNAL,
			CV_CHAIN_APPROX_SIMPLE);
	// Approximate contours to polygons + get bounding rects
	vector<vector<Point> > contours_poly(contours.size());

	for (int i = 0; i < contours.size(); i++)
	{
		approxPolyDP(Mat(contours[i]), contours_poly[i], 3, true);
		Rect bb = boundingRect(Mat(contours_poly[i]));

		if (bb.area() < m_minArea * m_Mat.cols * m_Mat.rows)
			continue;

		obj.m_iClass = 0;
		obj.m_bbox.m_x = bb.x;
		obj.m_bbox.m_y = bb.y;
		obj.m_bbox.m_z = bb.x + bb.width;
		obj.m_bbox.m_w = bb.y + bb.height;
		obj.m_camSize.m_x = gMat2.cols;
		obj.m_camSize.m_y = gMat2.rows;
		obj.m_dist = 0.0;
		obj.m_prob = 0.0;

		m_pUniverse->addObject(&obj);
//		m_pUniverse->addObject(m_pStream->getBGRFrame()->getCMat(), &bb, &contours_poly[i]);
	}

}

bool _Depth::draw(void)
{
	CHECK_F(!this->BASE::draw());
	Window* pWin = (Window*)this->m_pWindow;
	Frame* pFrame = pWin->getFrame();
	Mat* pMat = pFrame->getCMat();

	pFrame->update(m_pThrFrame);

	CHECK_F(!this->_ThreadBase::draw());
	pWin->lineNext();

	return true;
}

}
*/
