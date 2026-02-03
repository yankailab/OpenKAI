/*
 * _Lane.cpp
 *
 *  Created on: Mar 5, 2018
 *      Author: yankai
 */

#include "_Lane.h"

namespace kai
{

	_Lane::_Lane()
	{
		m_pV = nullptr;

		m_vRoiLT.x = 0.2;
		m_vRoiLT.y = 0.5;
		m_vRoiLB.x = 0.0;
		m_vRoiLB.y = 1.0;
		m_vRoiRT.x = 0.8;
		m_vRoiRT.y = 0.5;
		m_vRoiRB.x = 1.0;
		m_vRoiRB.y = 1.0;

		m_sizeOverhead.x = 400;
		m_sizeOverhead.y = 300;
		m_vSize.clear();
		m_binMed = 3;

		m_nFilter = 0;
		m_nLane = 0;
		m_ppPoint = NULL;
		m_pNp = NULL;
		m_bDrawOverhead = false;
		m_bDrawFilter = false;
	}

	_Lane::~_Lane()
	{
	}

	bool _Lane::init(const json &j)
	{
		IF_F(!this->_ModuleBase::init(j));

		jKv(j, "bDrawOverhead", m_bDrawOverhead);
		jKv(j, "bDrawFilter", m_bDrawFilter);
		jKv(j, "binMed", m_binMed);

		jKv<float>(j, "vRoiLT", m_vRoiLT);
		jKv<float>(j, "vRoiLB", m_vRoiLB);
		jKv<float>(j, "vRoiRT", m_vRoiRT);
		jKv<float>(j, "vRoiRB", m_vRoiRB);
		jKv(j, "overheadW", m_sizeOverhead.x);
		jKv(j, "overheadH", m_sizeOverhead.y);

		m_mOverhead = Mat(Size(m_sizeOverhead.x, m_sizeOverhead.y), CV_8UC3);

		// color filters
		m_nFilter = 0;
		const json &jC = jK(j, "colorFilters");
		if (jC.is_object())
		{
			for (auto it = jC.begin(); it != jC.end(); it++)
			{
				IF_F(m_nFilter >= N_LANE_FILTER);

				const json &Ji = it.value();
				IF_CONT(!Ji.is_object());

				LANE_FILTER *pF = &m_pFilter[m_nFilter];
				jKv(Ji, "iColorSpace", pF->m_iColorSpace);
				jKv(Ji, "iChannel", pF->m_iChannel);
				jKv(Ji, "nTile", pF->m_nTile);
				jKv(Ji, "thr", pF->m_thr);
				jKv(Ji, "clipLim", pF->m_clipLim);
				pF->init();

				m_nFilter++;
			}
		}

		// lanes
		int nAvr = 0;
		jKv(j, "nAvr", nAvr);
		int nMed = 0;
		jKv(j, "nMed", nMed);
		m_nLane = 0;
		const json &jL = jK(j, "lane");
		if (jL.is_object())
		{
			for (auto it = jL.begin(); it != jL.end(); it++)
			{
				IF_F(m_nLane >= N_LANE_FILTER);

				const json &Ji = it.value();
				IF_CONT(!Ji.is_object());

				LANE *pLane = &m_pLane[m_nLane];
				pLane->init(m_sizeOverhead.y, nAvr, nMed);
				jKv<float>(Ji, "vROI", pLane->m_ROI);

				m_nLane++;
			}
		}

		m_ppPoint = new Point *[m_nLane];
		m_pNp = new int[m_nLane];
		for (int i = 0; i < m_nLane; i++)
		{
			m_ppPoint[i] = new Point[m_sizeOverhead.y];
			m_pNp[i] = m_sizeOverhead.y;
		}

		return true;
	}

	bool _Lane::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_ModuleBase::link(j, pM));

		string n = "";
		jKv(j, "_VisionBase", n);
		m_pV = (_VisionBase *)(pM->findModule(n));
		NULL_F(m_pV);

		return true;
	}

	bool _Lane::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	bool _Lane::check(void)
	{
		NULL_F(m_pV);
		IF_F(m_pV->getFrameRGB()->m()->empty());

		return this->_ModuleBase::check();
	}

	void _Lane::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			detect();
		}
	}

	void _Lane::detect(void)
	{
		IF_(!check());
		Mat *pM = m_pV->getFrameRGB()->m();

		// Warp transform to get overhead view
		if (m_vSize.x != pM->cols || m_vSize.y != pM->rows)
		{
			m_vSize.x = pM->cols;
			m_vSize.y = pM->rows;
			updateVisionSize();
		}

		cv::warpPerspective(*pM, m_mOverhead, m_mPerspective,
							m_mOverhead.size(), cv::INTER_LINEAR);

		// Filter image to get binary view
		filterBin();

		for (int i = 0; i < m_nLane; i++)
		{
			LANE *pL = &m_pLane[i];
			pL->findLane(m_mBin);
			pL->updatePolyFit();
			pL->updateDeviation();
		}
	}

	void _Lane::filterBin(void)
	{
		Mat mAll = Mat::zeros(m_mOverhead.size(), CV_8UC1);

		for (int i = 0; i < m_nFilter; i++)
		{
			mAll += m_pFilter[i].filter(m_mOverhead);
		}

		cv::normalize(mAll, mAll, 0, 255, cv::NORM_MINMAX);
		cv::medianBlur(mAll, m_mBin, m_binMed);
	}

	void _Lane::updateVisionSize(void)
	{
		Point2f LT = Point2f((float)(m_vRoiLT.x * m_vSize.x),
							 (float)(m_vRoiLT.y * m_vSize.y));
		Point2f LB = Point2f((float)(m_vRoiLB.x * m_vSize.x),
							 (float)(m_vRoiLB.y * m_vSize.y));
		Point2f RT = Point2f((float)(m_vRoiRT.x * m_vSize.x),
							 (float)(m_vRoiRT.y * m_vSize.y));
		Point2f RB = Point2f((float)(m_vRoiRB.x * m_vSize.x),
							 (float)(m_vRoiRB.y * m_vSize.y));

		// LT, LB, RB, RT
		Point2f ptsFrom[] = {LT, LB, RB, RT};
		Point2f ptsTo[] =
			{cv::Point2f(0, 0),
			 cv::Point2f(0, (float)m_sizeOverhead.y),
			 cv::Point2f((float)m_sizeOverhead.x, (float)m_sizeOverhead.y),
			 cv::Point2f((float)m_sizeOverhead.x, 0)};

		m_mPerspective = getPerspectiveTransform(ptsFrom, ptsTo);
		m_mPerspectiveInv = getPerspectiveTransform(ptsTo, ptsFrom);
	}

	void _Lane::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);

		for (int i = 0; i < m_nLane; i++)
		{
			LANE *pL = &m_pLane[i];
			IF_CONT(!pL->m_pPoly);
			string msg = "Lane " + i2str(i) + ": " + f2str(pL->m_pPoly[0]) + " " + f2str(pL->m_pPoly[1]) + " " + f2str(pL->m_pPoly[2]);
			((_Console *)pConsole)->addMsg(msg, 1);
		}
	}

	void _Lane::draw(void *pFrame)
	{
		NULL_(pFrame);
		this->_ModuleBase::draw(pFrame);
		IF_(!check());

		Frame *pF = (Frame *)pFrame;
		Mat *pM = pF->m();
		IF_(pM->empty());

		IF_(m_mPerspectiveInv.empty());

		int i, j;
		LANE *pL;

		// visualization of the lane
		Mat mLane = Mat::zeros(m_mOverhead.size(), CV_8UC1);
		Mat mOverlay = Mat::zeros(pM->size(), CV_8UC1);

		for (i = 0; i < m_nLane; i++)
		{
			pL = &m_pLane[i];
			IF_CONT(!pL->m_pPoly);

			for (j = 0; j < m_sizeOverhead.y; j++)
			{
				m_ppPoint[i][j] = Point(pL->vPoly(j), j);
			}
		}

		polylines(mLane, m_ppPoint, m_pNp, m_nLane, false, cv::Scalar(255), 3, 4);
		cv::warpPerspective(mLane, mOverlay, m_mPerspectiveInv, mOverlay.size(), cv::INTER_LINEAR);

		vector<cv::Mat> vBGR;
		cv::split(*pM, vBGR);
		vBGR[0] -= mOverlay;
		vBGR[1] -= mOverlay;
		vBGR[2] += mOverlay;
		cv::merge(vBGR, *pM);

		if (m_bDrawOverhead && !m_mOverhead.empty())
		{
			Mat mO = m_mOverhead.clone();

			for (i = 0; i < m_nLane; i++)
			{
				pL = &m_pLane[i];
				IF_CONT(!pL->m_pPoly);

				for (j = 0; j < m_sizeOverhead.y; j++)
				{
					circle(mO, Point(pL->vPoly(j), j), 1, Scalar(0, 255, 0), 1);
					circle(mO, Point(pL->vFilter(j), j), 1, Scalar(0, 0, 255), 1);
				}
			}

			imshow(this->getName() + ":Overhead", mO);
		}

		if (m_bDrawOverhead && !m_mBin.empty())
		{
			imshow(this->getName() + ":Bin", m_mBin);
		}
	}

}
