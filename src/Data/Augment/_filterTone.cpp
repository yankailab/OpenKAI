/*
 * _filterTone.cpp
 *
 *  Created on: Mar 12, 2018
 *      Author: yankai
 */

#include "_filterTone.h"

namespace kai
{

_filterTone::_filterTone()
{
}

_filterTone::~_filterTone()
{
}

bool _filterTone::init(void* pKiss)
{
	IF_F(!this->_FilterBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	return true;
}

bool _filterTone::start(void)
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

void _filterTone::update(void)
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
			Mat A1 = Mat(mIn.size(), CV_32FC1, Scalar(1.0+m_dRand*NormRand()) );
			Mat A2 = Mat(mIn.size(), CV_32FC1, Scalar(1.0+m_dRand*NormRand()) );
			Mat A3 = Mat(mIn.size(), CV_32FC1, Scalar(1.0+m_dRand*NormRand()) );
		    Mat t[] = {A1, A2, A3};
		    Mat Afc3, Bfc3;
		    merge(t, 3, Afc3);
		    mIn.convertTo(Bfc3, CV_32FC3, 1/255.0);

		    Mat mMul = Afc3.mul(Bfc3);
			Mat mOut;
		    mMul.convertTo(mOut, mIn.type(), 255.0);

			cv::imwrite(dirNameIn + uuid() + m_extOut, mOut, m_PNGcompress);
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

