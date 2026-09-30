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
	}

	_HistEqualize::~_HistEqualize()
	{
	}

	bool _HistEqualize::loadConfig(void)
	{
		IF_F(!_RGBbase::loadConfig());

		return true;
	}

	bool _HistEqualize::saveConfig(bool bExport)
	{
		IF_F(!_RGBbase::saveConfig(false));

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool _HistEqualize::link(InstanceMgr *pM)
	{
		IF_F(!this->_RGBbase::link(pM));
		const json &j = *m_pJ;

		string n = "";
		jKv(j, "RGBframeIn", n);
		m_pRGBin = dynamic_cast<RGBframe *>(static_cast<DataObjBase *>(pM->findDataObject(n)));
		NULL_F(m_pRGBin);

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
		Mat mOut;
		NULL_(m_pRGBout);
		NULL_(m_pRGBin);
		Mat mRGB;
		const uint64_t tStamp = m_pRGBin->get(mRGB);
		IF_(mRGB.empty());

		Mat mIn;
		vector<Mat> vChannels;

		// Using reference code from:
		// https://opencv-srf.blogspot.jp/2013/08/histogram-equalization.html

		cv::cvtColor(mRGB, mIn, COLOR_BGR2YCrCb); // change the color image from BGR to YCrCb format
		split(mIn, vChannels);						  // split the image into channels
		cv::equalizeHist(vChannels[0], vChannels[0]); // equalize histogram on the 1st channel (Y)
		merge(vChannels, mIn);						  // merge 3 channels including the modified 1st channel into one image
		
		cv::cvtColor(mIn, mOut, COLOR_YCrCb2BGR);  // change the color image from YCrCb to BGR format (to display image properly)
		m_pRGBout->set(mOut, tStamp);
	}

}
