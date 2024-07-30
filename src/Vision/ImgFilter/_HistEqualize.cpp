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
		close();
	}

	int _HistEqualize::init(void *pKiss)
	{
		CHECK_(_VisionBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		string n;
		n = "";
		pK->v("_VisionBase", &n);
		m_pV = (_VisionBase *)(pK->findModule(n));
		NULL__(m_pV, OK_ERR_NOT_FOUND);

		return OK_OK;
	}

	bool _HistEqualize::open(void)
	{
		NULL_F(m_pV);
		m_bOpen = m_pV->isOpened();

		return m_bOpen;
	}

	void _HistEqualize::close(void)
	{
		this->_VisionBase::close();
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
			if (!m_bOpen)
				open();

			m_pT->autoFPSfrom();

			if (m_bOpen)
				filter();

			m_pT->autoFPSto();
		}
	}

	void _HistEqualize::filter(void)
	{
		IF_(m_pV->getFrameRGB()->bEmpty());

		Mat mIn;
		Mat mOut;
		vector<Mat> vChannels;

		//Using reference code from:
		//https://opencv-srf.blogspot.jp/2013/08/histogram-equalization.html

		cv::cvtColor(*m_pV->getFrameRGB()->m(), mIn, COLOR_BGR2YCrCb); //change the color image from BGR to YCrCb format
		split(mIn, vChannels);								   //split the image into channels
		cv::equalizeHist(vChannels[0], vChannels[0]);		   //equalize histogram on the 1st channel (Y)
		merge(vChannels, mIn);								   //merge 3 channels including the modified 1st channel into one image
		cv::cvtColor(mIn, mOut, COLOR_YCrCb2BGR);			   //change the color image from YCrCb to BGR format (to display image properly)

		m_fRGB.copy(mOut);
	}

}
