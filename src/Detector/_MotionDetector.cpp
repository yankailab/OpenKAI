/*
 * _MotionDetector.cpp
 *
 *  Created on: Aug 4, 2018
 *      Author: yankai
 */

#include "_MotionDetector.h"

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

	int _MotionDetector::init(void *pKiss)
	{
		CHECK_(this->_DetectorBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v<string>("algorithm", &m_algorithm);
		pK->v<double>("learningRate", &m_learningRate);

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

		string n = "";
		pK->v("_VisionBase", &n);
		m_pVision = (_VisionBase *)(pK->findModule(n));

		return OK_OK;
	}

	int _MotionDetector::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		return m_pT->start(getUpdate, this);
	}

	int _MotionDetector::check(void)
	{
		NULL__(m_pU, OK_ERR_NULLPTR);
		NULL__(m_pV, OK_ERR_NULLPTR);
		IF__(m_pV->getFrameRGB()->bEmpty(), OK_ERR_NULLPTR);

		return this->_DetectorBase::check();
	}

	void _MotionDetector::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPSfrom();

			m_pU->swap();
			detect();

			ON_PAUSE;
			m_pT->autoFPSto();
		}
	}

	void _MotionDetector::detect(void)
	{
		IF_(check() != OK_OK);

		Mat m = *m_pVision->getFrameRGB()->m();

		m_pBS->apply(m, m_mFG, m_learningRate);

		vector<vector<Point>> vContours;
		vector<Vec4i> vHierarchy;
		findContours(m_mFG, vContours, vHierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE,
					 Point(0, 0));

		float kx = 1.0 / m.cols;
		float ky = 1.0 / m.rows;
		_Object o;
		for (int i = 0; i < vContours.size(); i++)
		{
			vector<Point> vContourPoly;
			approxPolyDP(Mat(vContours[i]), vContourPoly, 3, true);
			Rect r = boundingRect(Mat(vContourPoly));
			vContourPoly.clear();

			o.clear();
			o.setTopClass(-1, 0);
			//		o.m_tStamp = m_pT->getTfrom();
			o.setBB2D(rect2BB<vFloat4>(r));
			o.scale(kx, ky);

			m_pU->add(o);
		}
	}

	void _MotionDetector::draw(void *pFrame)
	{
		NULL_(pFrame);
		this->_DetectorBase::draw(pFrame);
		IF_(check() != OK_OK);

		if (!m_mFG.empty())
		{
			imshow(this->getName(), m_mFG);
		}
	}

}
