/*
 * _MotionDetector.cpp
 *
 *  Created on: Aug 4, 2018
 *      Author: yankai
 */

#include "_MotionDetector.h"

#ifdef USE_OPENCV_CONTRIB

namespace kai
{

_MotionDetector::_MotionDetector()
{
	m_pVision = NULL;
	m_algorithm = "";
	m_learningRate = -1;
}

_MotionDetector::~_MotionDetector()
{

}

bool _MotionDetector::init(void* pKiss)
{
	IF_F(!this->_DetectorBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

	KISSm(pK, algorithm);
	KISSm(pK, learningRate);

//	if(m_algorithm == "cnt")
//	{
//		m_pBS = cv::bgsegm::createBackgroundSubtractorCNT();
//	}
//	else if(m_algorithm == "gmg")
//	{
//		m_pBS = cv::bgsegm::createBackgroundSubtractorGMG();
//	}
//	else if(m_algorithm == "lsbp")
//	{
//		m_pBS = cv::bgsegm::createBackgroundSubtractorLSBP();
//	}
//	else if(m_algorithm == "mog")
//	{
//		m_pBS = cv::bgsegm::createBackgroundSubtractorMOG();
//	}
//	else
//	{
//		m_pBS = cv::bgsegm::createBackgroundSubtractorGSOC();
//	}
//

	//link
	string iName = "";
	F_ERROR_F(pK->v("_VisionBase",&iName));
	m_pVision = (_VisionBase*)(pK->root()->getChildInst(iName));

	return true;
}

bool _MotionDetector::start(void)
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

void _MotionDetector::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		m_obj.update();
		detect();

		this->autoFPSto();
	}
}

void _MotionDetector::detect(void)
{
	NULL_(m_pVision);
	Mat m = *m_pVision->BGR()->m();
	IF_(m.empty());

    m_pBS->apply(m, m_mFG, m_learningRate);

	vector< vector< Point > > vContours;
	vector<Vec4i> vHierarchy;
	findContours(m_mFG, vContours, vHierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0));

	vInt2 cs;
	cs.x = m.cols;
	cs.y = m.rows;
	OBJECT obj;
	for( int i = 0; i < vContours.size(); i++ )
	{
		vector<Point> vContourPoly;
		approxPolyDP( Mat(vContours[i]), vContourPoly, 3, true );
	    Rect r = boundingRect( Mat(vContourPoly) );
	    vContourPoly.clear();

	    obj.init();
	    obj.setTopClass(-1,0);
	    obj.m_tStamp = m_tStamp;
		obj.setBB(r,cs);

	    add(&obj);
	}
}

bool _MotionDetector::draw(void)
{
	IF_F(!this->_DetectorBase::draw());
	Window* pWin = (Window*)this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();

	if(!m_mFG.empty())
	{
		imshow(*this->getName(), m_mFG);
	}

	return true;
}

bool _MotionDetector::console(int& iY)
{
	IF_F(!this->_DetectorBase::console(iY));

	return true;
}

}
#endif
