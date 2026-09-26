/*
 * _HistEqualize.cpp
 *
 *  Created on: March 12, 2019
 *      Author: yankai
 */

#include "_HistEqualize.h"

namespace kai
{

	_HistEqualize::_HistEqualize()
	{
		m_type = vision_histEqualize;
	}

	_HistEqualize::~_HistEqualize()
	{
	}

	bool _HistEqualize::loadConfig(void)
	{
		IF_F(!_VisionBase::loadConfig());

		return true;
	}

	bool _HistEqualize::saveConfig(bool bExport)
	{
		if (!_VisionBase::saveConfig(false))
		{
			return false;
		}

		if (!bExport)
		{
			return true;
		}
		return m_pJcfg->saveToFile();
	}

	bool _HistEqualize::link(void)
	{
		IF_F(!this->_VisionBase::link());
		const json &j = *m_pJ;

		string n = "";
		jKv(j, "_VisionBase", n);
		m_pV = (_VisionBase *)(m_pM->findModule(n));
		NULL_F(m_pV);

		return true;
	}

	bool _HistEqualize::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	void _HistEqualize::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPS();

			filter();
		}
	}

	void _HistEqualize::filter(void)
	{
		NULL_(m_pV);
		Mat mRGB;
		m_pV->copyMatRGB(mRGB);
		IF_(mRGB.empty());

		Mat mIn;
		vector<Mat> vChannels;

		// Using reference code from:
		// https://opencv-srf.blogspot.jp/2013/08/histogram-equalization.html

		cv::cvtColor(mRGB, mIn, COLOR_BGR2YCrCb); // change the color image from BGR to YCrCb format
		split(mIn, vChannels);						  // split the image into channels
		cv::equalizeHist(vChannels[0], vChannels[0]); // equalize histogram on the 1st channel (Y)
		merge(vChannels, mIn);						  // merge 3 channels including the modified 1st channel into one image
		
		std::lock_guard<std::mutex> lock(m_mutexRGB);
		cv::cvtColor(mIn, m_mRGB, COLOR_YCrCb2BGR);  // change the color image from YCrCb to BGR format (to display image properly)
	}

}
