/*
 * _D2RGB.cpp
 *
 *  Created on: April 23, 2019
 *      Author: yankai
 */

#include "_D2RGB.h"

namespace kai
{

	_D2RGB::_D2RGB()
	{
		m_type = vision_D2RGB;
	}

	_D2RGB::~_D2RGB()
	{
	}

	bool _D2RGB::loadConfig(void)
	{
		IF_F(!_RGBDbase::loadConfig());
		const json &j = *m_pJ;

		jKv(j, "nHistLev", m_nHistLev);
		jKv(j, "iHistFrom", m_iHistFrom);
		jKv(j, "minHistD", m_minHistD);
		jKv(j, "bMeasure", m_bMeasure);

		return true;
	}

	bool _D2RGB::saveConfig(bool bExport)
	{
		if (!_RGBDbase::saveConfig(false))
		{
			return false;
		}

		json &j = *m_pJ;
		j["nHistLev"] = m_nHistLev;
		j["iHistFrom"] = m_iHistFrom;
		j["minHistD"] = m_minHistD;
		j["bMeasure"] = m_bMeasure;

		if (!bExport)
		{
			return true;
		}
		return m_pJcfg->saveToFile();
	}

	bool _D2RGB::link(void)
	{
		IF_F(!this->_RGBDbase::link());
		const json &j = *m_pJ;

		string n = "";
		jKv(j, "_RGBDbase", n);
		m_pVd = (_RGBDbase *)(m_pM->findModule(n));
		NULL_F(m_pVd);

		return true;
	}

	bool _D2RGB::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	void _D2RGB::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPS();

			filter();
		}
	}

	void _D2RGB::filter(void)
	{
		NULL_(m_pVd);
		Mat mDepth;
		m_pVd->copyMatDepth(mDepth);
		IF_(mDepth.empty());

		// Measurements and drawing share these depth and RGB buffers.
		Mat mGray;

		{
			std::lock_guard<std::mutex> lock(m_mutexDepth);
			if (mDepth.type() == CV_16UC1)
			{
				mDepth.convertTo(m_mDepth, CV_32FC1, m_pVd->getDepthScale(), m_pVd->getDepthOffset());
			}
			else if (mDepth.type() == CV_32FC1)
			{
				mDepth.copyTo(m_mDepth);
			}
			else
			{
				return;
			}

			cv::normalize(m_mDepth, mGray, 0, 255, cv::NORM_MINMAX, CV_8UC1);
		}

		{
			std::lock_guard<std::mutex> lock(m_mutexRGB);
			cv::applyColorMap(mGray, m_mRGB, cv::COLORMAP_JET);
		}
	}

	float _D2RGB::d(const Vector4i &bb)
	{
		Rect r = bb2Rect(bb);
		Mat mRoi;
		{
			std::lock_guard<std::mutex> lock(m_mutexDepth);
			IF__(m_mDepth.empty(), -1);

			mRoi = m_mDepth(r);
		}

		Vector2f vRangeD = m_pVd->getDepthRange();
		vector<int> vHistLev = {m_nHistLev};
		vector<float> vRange = {vRangeD.x(), vRangeD.y()};
		vector<int> vChannel = {0};

		vector<Mat> vRoi = {mRoi};
		Mat mHist;
		cv::calcHist(vRoi, vChannel, Mat(),
					 mHist, vHistLev, vRange,
					 false // accumulate
		);

		int nMinHist = m_minHistD * mRoi.cols * mRoi.rows;
		int nPix = 0;
		int i;
		for (i = m_iHistFrom; i < m_nHistLev; i++)
		{
			nPix += (int)mHist.at<float>(i);
			if (nPix >= nMinHist)
				break;
		}

		return (vRangeD.x() + (((float)i) / (float)m_nHistLev) * vRangeD.norm());
	}

	void _D2RGB::draw(void *pMat)
	{
		NULL_(pMat);
		this->_RGBDbase::draw(pMat);
		IF_(!check());

		if (m_bMeasure)
		{
			Mat *pM = static_cast<Mat *>(pMat);
			IF_(pM->empty());

			Vector4f vRoi(0.4, 0.4, 0.6, 0.6);
			Vector4i bb = Vector4i::Zero();
			bb.x() = vRoi.x() * pM->cols;
			bb.y() = vRoi.y() * pM->rows;
			bb.z() = vRoi.z() * pM->cols;
			bb.w() = vRoi.w() * pM->rows;
			Rect r = bb2Rect(bb);
			rectangle(*pM, r, Scalar(128, 128, 128), 2);

			putText(*pM, f2str(d(bb)),
					Point(r.x + 15, r.y + 25),
					FONT_HERSHEY_SIMPLEX, 0.6, Scalar(128, 128, 128), 2);
		}
	}

}
