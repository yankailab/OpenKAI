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

	bool _D2RGB::init(const json &j)
	{
		IF_F(!_VisionBase::init(j));

		jKv(j, "nHistLev", m_nHistLev);
		jKv(j, "iHistFrom", m_iHistFrom);
		jKv(j, "minHistD", m_minHistD);
		jKv(j, "bMeasure", m_bMeasure);

		return true;
	}

	bool _D2RGB::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_VisionBase::link(j, pM));

		string n = "";
		jKv(j, "_RGBDbase", n);
		m_pV = (_RGBDbase *)(pM->findModule(n));
		NULL_F(m_pV);

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
		NULL_(m_pV);
		Mat *pM = m_pV->getMatDepth();
		NULL_(pM);
		IF_(pM->empty());

		Mat mGray;
		if (pM->type() == CV_16UC1)
		{
			pM->convertTo(m_mDreal, CV_32FC1, m_pV->getDepthScale(), m_pV->getDepthOffset());
		}
		else if (pM->type() == CV_32FC1)
		{
			pM->copyTo(m_mDreal);
		}
		else
		{
			return;
		}

		cv::normalize(m_mDreal, mGray, 0, 255, cv::NORM_MINMAX, CV_8UC1);

		cv::applyColorMap(mGray, m_mRGB, cv::COLORMAP_JET);
	}

	float _D2RGB::d(const Vector4f &bb)
	{
		IF__(m_mDreal.empty(), -1);

		Size s = m_mDreal.size();
		Vector4i vBB = Vector4i::Zero();
		vBB.x() = bb.x() * s.width;
		vBB.y() = bb.y() * s.height;
		vBB.z() = bb.z() * s.width;
		vBB.w() = bb.w() * s.height;

		if (vBB.x() < 0)
			vBB.x() = 0;
		if (vBB.y() < 0)
			vBB.y() = 0;
		if (vBB.z() > s.width)
			vBB.z() = s.width;
		if (vBB.w() > s.height)
			vBB.w() = s.height;

		return d(vBB);
	}

	float _D2RGB::d(const Vector4i &bb)
	{
		IF__(m_mDreal.empty(), -1);

		Vector2f vRangeD = m_pV->getDepthRange();

		vector<int> vHistLev = {m_nHistLev};
		vector<float> vRange = {vRangeD.x(), vRangeD.y()};
		vector<int> vChannel = {0};

		Rect r = bb2Rect(bb);
		Mat mRoi = m_mDreal(r);
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
		this->_VisionBase::draw(pMat);
		IF_(!check());
		IF_(m_mRGB.empty());

		if (m_bMeasure)
		{
			Mat *pM = static_cast<Mat *>(pMat);
			IF_(pM->empty());

			Vector4f vRoi(0.4, 0.4, 0.6, 0.6);

			Vector4f bb = Vector4f::Zero();
			bb.x() = vRoi.x() * pM->cols;
			bb.y() = vRoi.y() * pM->rows;
			bb.z() = vRoi.z() * pM->cols;
			bb.w() = vRoi.w() * pM->rows;
			Rect r = bb2Rect(bb);
			rectangle(*pM, r, Scalar(128, 128, 128), 2);

			putText(*pM, f2str(d(vRoi)),
					Point(r.x + 15, r.y + 25),
					FONT_HERSHEY_SIMPLEX, 0.6, Scalar(128, 128, 128), 2);
		}
	}

}
