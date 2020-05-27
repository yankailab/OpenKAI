/*
 * _HistEqualize.cpp
 *
 *  Created on: March 12, 2019
 *      Author: yankai
 */

#include "_HistEqualize.h"

#ifdef USE_OPENCV

namespace kai
{

_HistEqualize::_HistEqualize()
{
	m_type = vision_histEqualize;
	m_pV = NULL;
}

_HistEqualize::~_HistEqualize()
{
	close();
}

bool _HistEqualize::init(void* pKiss)
{
	IF_F(!_VisionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	string iName;
	iName = "";
	pK->v("_VisionBase", &iName);
	m_pV = (_VisionBase*) (pK->parent()->getChildInst(iName));
	IF_Fl(!m_pV, iName + ": not found");

	return true;
}

bool _HistEqualize::open(void)
{
	NULL_F(m_pV);
	m_bOpen = m_pV->isOpened();

	return m_bOpen;
}

void _HistEqualize::close(void)
{
	if(m_threadMode==T_THREAD)
	{
		goSleep();
		while(!bSleeping());
	}

	this->_VisionBase::close();
}

bool _HistEqualize::start(void)
{
	IF_F(!this->_ThreadBase::start());

	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _HistEqualize::update(void)
{
	while (m_bThreadON)
	{
		if (!m_bOpen)
			open();

		this->autoFPSfrom();

		if(m_bOpen)
			filter();

		this->autoFPSto();
	}
}

void _HistEqualize::filter(void)
{
	IF_(m_pV->BGR()->bEmpty());

	Mat mIn;
	Mat mOut;
    vector<Mat> vChannels;

    //Using reference code from:
    //https://opencv-srf.blogspot.jp/2013/08/histogram-equalization.html

	cv::cvtColor(*m_pV->BGR()->m(), mIn, COLOR_BGR2YCrCb);	//change the color image from BGR to YCrCb format
	split(mIn, vChannels); 							//split the image into channels
	cv::equalizeHist(vChannels[0], vChannels[0]);   //equalize histogram on the 1st channel (Y)
	merge(vChannels, mIn); 							//merge 3 channels including the modified 1st channel into one image
    cv::cvtColor(mIn, mOut, COLOR_YCrCb2BGR); 		//change the color image from YCrCb to BGR format (to display image properly)

	m_fBGR.copy(mOut);
}

}
#endif
