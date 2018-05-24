/*
 * _filterLowResolution.cpp
 *
 *  Created on: Mar 12, 2018
 *      Author: yankai
 */

#include "_filterLowResolution.h"

namespace kai
{

_filterLowResolution::_filterLowResolution()
{
	m_minScale = 0.5;
}

_filterLowResolution::~_filterLowResolution()
{
}

bool _filterLowResolution::init(void* pKiss)
{
	IF_F(!this->_FilterBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	KISSdm(pK, minScale);

	return true;
}

bool _filterLowResolution::link(void)
{
	IF_F(!this->_DataBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	return true;
}

bool _filterLowResolution::start(void)
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

void _filterLowResolution::update(void)
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
			double scale = m_minScale + m_dRand * NormRand();
			Frame fIn;
			fIn = mIn;
			Frame fOut = fIn.resize(scale, scale);
			Mat mOut = *fOut.m();

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

