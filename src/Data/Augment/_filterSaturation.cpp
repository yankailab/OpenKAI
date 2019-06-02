/*
 * _filterSaturation.cpp
 *
 *  Created on: Mar 12, 2018
 *      Author: yankai
 */

#include "_filterSaturation.h"

namespace kai
{

_filterSaturation::_filterSaturation()
{
	m_dSaturation = 10;
}

_filterSaturation::~_filterSaturation()
{
}

bool _filterSaturation::init(void* pKiss)
{
	IF_F(!this->_FilterBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK,dSaturation);

	return true;
}

bool _filterSaturation::start(void)
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

void _filterSaturation::update(void)
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

		for (int j = 0; j < m_nProduce; j++)
		{
			// what it does here is dst = (uchar) ((double)src*scale+saturation);
			Mat mOut;
			mIn.convertTo(mOut, mIn.type(), m_dRand*NormRand(), m_dSaturation*NormRand());

			cv::imwrite(dirNameIn + uuid() + m_extOut, mOut, m_vCompress);
			nTot++;
		}

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

