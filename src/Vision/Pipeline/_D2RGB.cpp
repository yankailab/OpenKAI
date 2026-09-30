/*
 * _D2RGB.cpp
 *
 *  Created on: April 23, 2019
 *      Author: yankai
 */

#include "_D2RGB.h"
#include "../../Utility/utilCV.h"

namespace kai
{

	_D2RGB::_D2RGB()
	{
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
		m_tLastInput = 0;

		return true;
	}

	bool _D2RGB::saveConfig(bool bExport)
	{
		IF_F(!_RGBDbase::saveConfig(false));

		json &j = *m_pJ;
		j["nHistLev"] = m_nHistLev;
		j["iHistFrom"] = m_iHistFrom;
		j["minHistD"] = m_minHistD;
		j["bMeasure"] = m_bMeasure;

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool _D2RGB::link(InstanceMgr *pM)
	{
		IF_F(!this->_RGBDbase::link(pM));
		const json &j = *m_pJ;

		string n = "";
		jKv(j, "DframeIn", n);
		m_pDin = dynamic_cast<RGBframe *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
		NULL_F(m_pDin);

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
		NULL_(m_pDin);

		Mat mDepth;
		const uint64_t tStamp = m_pDin->get(mDepth);
		IF_(!tStamp || tStamp <= m_tLastInput || mDepth.empty() || mDepth.type() != CV_32FC1);

		Mat mGray;
		Mat mRGB;
		cv::normalize(mDepth, mGray, 0, 255, cv::NORM_MINMAX, CV_8UC1);
		cv::applyColorMap(mGray, mRGB, cv::COLORMAP_JET);

		if (m_pRGBout)
		{
			m_pRGBout->set(mRGB, tStamp);
		}

		if (m_pDout)
		{
			m_pDout->set(mDepth, tStamp);
		}

		if (m_pRGBDout)
		{
			m_pRGBDout->set(mRGB, mDepth, tStamp);
		}

		m_tLastInput = tStamp;
	}

	float _D2RGB::d(const Vector4i &bb)
	{
		NULL__(m_pDin, -1);
		Mat mDepth;
		m_pDin->get(mDepth);
		IF__(mDepth.empty() || mDepth.type() != CV_32FC1, -1);
		IF__(m_nHistLev <= 0 || m_iHistFrom < 0 || m_iHistFrom >= m_nHistLev, -1);
		IF__(!m_vRangeD.allFinite() || m_vRangeD.x() >= m_vRangeD.y(), -1);

		Rect r = bb2Rect(bb) & Rect(0, 0, mDepth.cols, mDepth.rows);
		IF__(r.empty(), -1);
		Mat mRoi = mDepth(r);
		Vector2f vRangeD = m_vRangeD;
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
			{
				break;
			}
		}

		return (vRangeD.x() + (((float)i) / (float)m_nHistLev) * (vRangeD.y() - vRangeD.x()));
	}

	void _D2RGB::draw(void *pMat)
	{
		NULL_(pMat);
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
