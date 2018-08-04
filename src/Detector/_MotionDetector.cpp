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
	m_minArea = DBL_MIN;
	m_maxArea = DBL_MAX;
}

_MotionDetector::~_MotionDetector()
{

}

bool _MotionDetector::init(void* pKiss)
{
	IF_F(!this->_ObjectBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;
	pK->m_pInst = this;

//	KISSm(pK, dict);

	m_pBS = cv::bgsegm::createBackgroundSubtractorGSOC();
    //cv::Ptr<cv::BackgroundSubtractor> bgfs = cv::bgsegm::createBackgroundSubtractorCNT();
    //cv::Ptr<cv::BackgroundSubtractor> bgfs = cv::bgsegm::createBackgroundSubtractorGMG();
    //cv::Ptr<cv::BackgroundSubtractor> bgfs = cv::bgsegm::createBackgroundSubtractorLSBP();
    //cv::Ptr<cv::BackgroundSubtractor> bgfs = cv::bgsegm::createBackgroundSubtractorMOG();
    //cv::Ptr<cv::BackgroundSubtractor> bgfs = cv::createBackgroundSubtractorMOG2();
    //cv::Ptr<cv::BackgroundSubtractor> bgfs = cv::createBackgroundSubtractorKNN();

	return true;
}

bool _MotionDetector::link(void)
{
	IF_F(!this->_ObjectBase::link());
	Kiss* pK = (Kiss*)m_pKiss;

	string iName = "";
	F_ERROR_F(pK->v("_VisionBase",&iName));
	m_pVision = (_VisionBase*)(pK->root()->getChildInstByName(&iName));

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

		this->_ObjectBase::update();
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

	Mat mFG, mSegm;
    m_pBS->apply(m, mFG);
    mFG.convertTo(mSegm, 0, 0.5);
	cv::add(m, cv::Scalar(100, 100, 0), mSegm, mFG);

//    std::vector<int> vID;
//    std::vector<std::vector<cv::Point2f> > vvCorner;
//    cv::aruco::detectMarkers(m, m_pDict, vvCorner, vID);
//
//	OBJECT obj;
//	double dx,dy;
//
//	for (int i = 0; i < vID.size(); i++)
//	{
//		obj.init();
//		obj.m_tStamp = m_tStamp;
//		obj.setTopClass(vID[i]);
//
//		Point2f pLT = vvCorner[i][0];
//		Point2f pRT = vvCorner[i][1];
//		Point2f pRB = vvCorner[i][2];
//		Point2f pLB = vvCorner[i][3];
//
//		// center position
//		obj.m_fBBox.x = (double)(pLT.x + pRT.x + pRB.x + pLB.x)*0.25;
//		obj.m_fBBox.y = (double)(pLT.y + pRT.y + pRB.y + pLB.y)*0.25;
//
//		// radius
//		dx = obj.m_fBBox.x - pLT.x;
//		dy = obj.m_fBBox.y - pLT.y;
//		obj.m_fBBox.z = sqrt(dx*dx + dy*dy);
//
//		// angle in deg
//		dx = pLB.x - pLT.x;
//		dy = pLB.y - pLT.y;
//		obj.m_fBBox.w = -atan2(dx,dy) * RAD_DEG;
//
//		obj.m_camSize.x = m.cols;
//		obj.m_camSize.y = m.rows;
//
//		add(&obj);
//		LOG_I("ID: "+ i2str(obj.m_iClass));
//	}
}

bool _MotionDetector::draw(void)
{
	IF_F(!this->_ThreadBase::draw());
	Window* pWin = (Window*)this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();

	string msg = "nTag: " + i2str(this->size());
	pWin->addMsg(&msg);
	IF_T(this->size() <= 0);

	OBJECT* pO;
	int i=0;
	while((pO = m_obj.at(i++)) != NULL)
	{
		Point pCenter = Point(pO->m_fBBox.x, pO->m_fBBox.y);
		circle(*pMat, pCenter, pO->m_fBBox.z, Scalar(0, 255, 0), 2);

		putText(*pMat, i2str(pO->m_iClass) + " / " + i2str(pO->m_fBBox.w),
				pCenter,
				FONT_HERSHEY_SIMPLEX, 0.5, Scalar(255, 0, 0), 2);

		double rad = -pO->m_fBBox.w * DEG_RAD;
		Point pD = Point(pO->m_fBBox.z*sin(rad), pO->m_fBBox.z*cos(rad));
		line(*pMat, pCenter + pD, pCenter - pD, Scalar(0, 0, 255), 2);
	}

	return true;
}

bool _MotionDetector::cli(int& iY)
{
	IF_F(!this->_ObjectBase::cli(iY));

	string msg = "| ";
	OBJECT* pO;
	int i=0;
	while((pO = m_obj.at(i++)) != NULL)
	{
		msg += i2str(pO->m_iClass) + " | ";
	}

	COL_MSG;
	iY++;
	mvaddstr(iY, CLI_X_MSG, msg.c_str());

	return true;
}

}
#endif
