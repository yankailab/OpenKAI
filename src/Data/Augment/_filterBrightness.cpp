/*
 * _filterBrightness.cpp
 *
 *  Created on: Mar 12, 2018
 *      Author: yankai
 */

#include "_filterBrightness.h"

namespace kai
{

_filterBrightness::_filterBrightness()
{
}

_filterBrightness::~_filterBrightness()
{
}

bool _filterBrightness::init(void* pKiss)
{
	IF_F(!this->_FilterBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	return true;
}

bool _filterBrightness::start(void)
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

void _filterBrightness::update(void)
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
		for (int j = 0; j < m_nProduce; j++)
		{
			mIn.convertTo(mOut, -1, 1, m_dRand * (NormRand() * 2.0 - 1.0));
			cv::imwrite(dirNameIn + uuid() + m_extOut, mOut, m_vCompress);
		}

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

