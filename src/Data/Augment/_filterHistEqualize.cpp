/*
 * _filterHistEqualize.cpp
 *
 *  Created on: Mar 12, 2018
 *      Author: yankai
 */

#include "_filterHistEqualize.h"

namespace kai
{

_filterHistEqualize::_filterHistEqualize()
{
}

_filterHistEqualize::~_filterHistEqualize()
{
}

bool _filterHistEqualize::init(void* pKiss)
{
	IF_F(!this->_FilterBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	return true;
}

bool _filterHistEqualize::start(void)
{
	m_bComplete = false;

	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _filterHistEqualize::update(void)
{
	srand(time(NULL));
	cv::RNG gen(cv::getTickCount());

	int nTot = 0;
	m_progress = 0.0;

	for (int i = 0; i < m_vFileIn.size(); i++)
	{
		string fNameIn = m_vFileIn[i];
		string dirNameIn = getFileDir(fNameIn);
		Mat mIn = cv::imread(fNameIn.c_str());
		IF_CONT(mIn.empty());

		Mat mOut;
        vector<Mat> vChannels;

        //Using reference code from:
        //https://opencv-srf.blogspot.jp/2013/08/histogram-equalization.html

		cv::cvtColor(mIn, mOut, COLOR_BGR2YCrCb); 			//change the color image from BGR to YCrCb format
		split(mOut, vChannels); 						//split the image into channels
		cv::equalizeHist(vChannels[0], vChannels[0]);   //equalize histogram on the 1st channel (Y)
		merge(vChannels,mIn); 							//merge 3 channels including the modified 1st channel into one image
        cv::cvtColor(mIn, mOut, COLOR_YCrCb2BGR); 			//change the color image from YCrCb to BGR format (to display image properly)

		cv::imwrite(dirNameIn + uuid() + m_extOut, mOut, m_PNGcompress);

		nTot++;
		double prog = (double) i / (double) m_vFileIn.size();
		if (prog - m_progress > 0.1)
		{
			m_progress = prog;
			LOG_I("  - Progress: " + i2str((int)(m_progress * 100)) + "%");
		}
	}

	LOG_I("  - Complete: Total produced: " + i2str(nTot));
	m_bComplete = true;
}

}

