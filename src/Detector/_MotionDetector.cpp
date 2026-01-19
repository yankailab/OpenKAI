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

	bool _MotionDetector::init(const json &j)
	{
		IF_F(!this->_DetectorBase::init(j));

		jKv(j, "algorithm", m_algorithm);
		jKv(j, "learningRate", m_learningRate);

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

		return true;
	}

	bool _MotionDetector::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_ModuleBase::link(j, pM));

		string n = "";
		jKv(j, "_VisionBase", n);
		m_pVision = (_VisionBase *)(pM->findModule(n));
		NULL_F(m_pVision);

		return true;
	}

	bool _MotionDetector::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	bool _MotionDetector::check(void)
	{
		NULL_F(m_pU);
		NULL_F(m_pV);
		IF_F(m_pV->getFrameRGB()->bEmpty());

		return this->_DetectorBase::check();
	}

	void _MotionDetector::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			m_pU->swap();
			detect();

			ON_PAUSE;
		}
	}

	void _MotionDetector::detect(void)
	{
		IF_(!check());

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
			o.setBB2D(rect2BB<vFloat4>(r), kx, ky);

			m_pU->add(o);
		}
	}

	void _MotionDetector::draw(void *pFrame)
	{
		NULL_(pFrame);
		this->_DetectorBase::draw(pFrame);
		IF_(!check());

		if (!m_mFG.empty())
		{
			imshow(this->getName(), m_mFG);
		}
	}

}
