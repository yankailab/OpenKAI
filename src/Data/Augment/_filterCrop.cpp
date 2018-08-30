/*
 * _filterCrop.cpp
 *
 *  Created on: Mar 12, 2018
 *      Author: yankai
 */

#include "_filterCrop.h"

namespace kai
{

_filterCrop::_filterCrop()
{
}

_filterCrop::~_filterCrop()
{
}

bool _filterCrop::init(void* pKiss)
{
	IF_F(!this->_FilterBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	return true;
}

bool _filterCrop::start(void)
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

void _filterCrop::update(void)
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
			double rScaleW = 1.0 - m_dRand * NormRand();
			int dSizeW = ((double) mIn.cols) * rScaleW;
			int dRandX = ((double) mIn.cols - dSizeW) * NormRand();

			double rScaleH = 1.0 - m_dRand * NormRand();
			int dSizeH = ((double) mIn.rows) * rScaleH;
			int dRandY = ((double) mIn.rows - dSizeH) * NormRand();

			Mat mOut = mIn(cv::Rect(dRandX, dRandY, dSizeW, dSizeH));

			cv::imwrite(dirNameIn + uuid() + m_extOut, mOut, m_PNGcompress);
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

