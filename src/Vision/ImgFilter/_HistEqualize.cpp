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
		m_pV = nullptr;
	}

	_HistEqualize::~_HistEqualize()
	{
	}

	int _HistEqualize::init(void *pKiss)
	{
		CHECK_(_VisionBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		return OK_OK;
	}

	int _HistEqualize::link(void)
	{
		CHECK_(this->_VisionBase::link());
		Kiss *pK = (Kiss *)m_pKiss;

		string n;
		n = "";
		pK->v("_VisionBase", &n);
		m_pV = (_VisionBase *)(pK->findModule(n));
		NULL__(m_pV, OK_ERR_NOT_FOUND);

		return OK_OK;
	}

	int _HistEqualize::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		return m_pT->start(getUpdate, this);
	}

	void _HistEqualize::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			filter();
		}
	}

	void _HistEqualize::filter(void)
	{
		NULL_(m_pV);
		Frame *pF = m_pV->getFrameRGB();
		NULL_(pF);
		IF_(pF->bEmpty());
		IF_(m_fRGB.tStamp() >= pF->tStamp());

		Mat mIn;
		Mat mOut;
		vector<Mat> vChannels;

		// Using reference code from:
		// https://opencv-srf.blogspot.jp/2013/08/histogram-equalization.html

		cv::cvtColor(*pF->m(), mIn, COLOR_BGR2YCrCb); // change the color image from BGR to YCrCb format
		split(mIn, vChannels);						  // split the image into channels
		cv::equalizeHist(vChannels[0], vChannels[0]); // equalize histogram on the 1st channel (Y)
		merge(vChannels, mIn);						  // merge 3 channels including the modified 1st channel into one image
		cv::cvtColor(mIn, mOut, COLOR_YCrCb2BGR);	  // change the color image from YCrCb to BGR format (to display image properly)

		m_fRGB.copy(mOut);
	}

}
