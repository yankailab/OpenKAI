/*
 * _filterShrink.cpp
 *
 *  Created on: Mar 12, 2018
 *      Author: yankai
 */

#include "_filterShrink.h"

namespace kai
{

_filterShrink::_filterShrink()
{
}

_filterShrink::~_filterShrink()
{
	reset();
}

bool _filterShrink::init(void* pKiss)
{
	IF_F(!this->_FilterBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	return true;
}

bool _filterShrink::link(void)
{
	IF_F(!this->_DataBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	return true;
}

bool _filterShrink::start(void)
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

void _filterShrink::update(void)
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
			Size s = mIn.size();
			Mat mMask = Mat::zeros(s, mIn.type());
			Mat mNoise = Mat::zeros(s, mIn.type());
			gen.fill(mNoise, m_noiseType,
					cv::Scalar(m_noiseMean, m_noiseMean, m_noiseMean),
					cv::Scalar(m_noiseDev, m_noiseDev, m_noiseDev));

			Frame fIn;
			fIn.copy(mIn);
			Frame fOut = fIn.resize(1.0 - m_dRand * NormRand(), 1.0 - m_dRand * NormRand());

			Mat sMat = *fOut.m();
			Mat mOut = mNoise;
			sMat.copyTo(mOut(cv::Rect((int) ((double) (mOut.cols - sMat.cols) * NormRand()),
									  (int) ((double) (mOut.rows - sMat.rows) * NormRand()),
									sMat.cols,
									sMat.rows)));

			cv::imwrite(dirNameIn + uuid() + m_extOut, mOut, m_PNGcompress);
		}

		nTot++;
		double prog = (double) i / (double) m_vFileIn.size();
		if (prog - m_progress > 0.1)
		{
			m_progress = prog;
			LOG_I("  - Progress: " << (int)(m_progress * 100) << "%");
		}
	}

	LOG_I("  - Complete: Total produced: " << nTot);
	m_bComplete = true;
}

}

