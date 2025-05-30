/*
 * _DepthSegment.cpp
 *
 *  Created on: May 29, 2019
 *      Author: yankai
 */

#include "_DepthSegment.h"

namespace kai
{

	_DepthSegment::_DepthSegment()
	{
		m_rL = 0.0;
		m_rH = 1.0;
		m_rD = 0.1;
		m_rArea = 0.8;
	}

	_DepthSegment::~_DepthSegment()
	{
	}

	int _DepthSegment::init(void *pKiss)
	{
		CHECK_(this->_DetectorBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v<float>("rL", &m_rL);
		pK->v<float>("rH", &m_rH);
		pK->v<float>("rD", &m_rD);
		pK->v<float>("rArea", &m_rArea);

		return OK_OK;
	}

	int _DepthSegment::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		return m_pT->start(getUpdate, this);
	}

	int _DepthSegment::check(void)
	{
		NULL__(m_pV, OK_ERR_NULLPTR);
		NULL__(m_pU, OK_ERR_NULLPTR);
		IF__(m_pV->getFrameRGB()->bEmpty(), OK_ERR_NULLPTR);

		return this->_DetectorBase::check();
	}

	void _DepthSegment::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			detect();

			ON_PAUSE;
		}
	}

	void _DepthSegment::detect(void)
	{
		IF_(check() != OK_OK);

		Mat m;
		m_pV->getFrameRGB()->m()->copyTo(m);

		float kx = 1.0 / m.cols;
		float ky = 1.0 / m.rows;

		vector<vector<Point>> vvC;
		for (float r = m_rL; r < m_rH; r += m_rD)
		{
			cv::inRange(m, 1, (r - m_rL) * 255.0 + 2, m_mR);
			findContours(m_mR, vvC, RETR_EXTERNAL, CHAIN_APPROX_NONE);

			_Object o;
			vector<Point> vPoly;
			float rArea = 0.0;
			for (unsigned int i = 0; i < vvC.size(); i++)
			{
				vPoly.clear();
				approxPolyDP(vvC[i], vPoly, 3, true);
				Rect re = boundingRect(vPoly);

				o.clear();
				//			o.m_tStamp = m_pT->getTfrom();
				o.setBB2D(rect2BB<vFloat4>(re));
				o.scale(kx, ky);
				o.setZ(r);
				o.setTopClass(0, o.area());

				// TODO: classify

				m_pU->add(o);
				rArea += o.area();
			}

			if (rArea > m_rArea)
				break;
		}

		m_pU->swap();
	}

	void _DepthSegment::draw(void *pFrame)
	{
		NULL_(pFrame);
		this->_DetectorBase::draw(pFrame);
		IF_(check() != OK_OK);

		if (!m_mR.empty())
		{
			imshow(this->getName() + ":Thr", m_mR);
		}
	}

}
