/*
 * _filterRotate.cpp
 *
 *  Created on: Mar 12, 2018
 *      Author: yankai
 */

#include "_filterRotate.h"

namespace kai
{

_filterRotate::_filterRotate()
{
}

_filterRotate::~_filterRotate()
{
	reset();
}

bool _filterRotate::init(void* pKiss)
{
	IF_F(!this->_FilterBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	return true;
}

bool _filterRotate::link(void)
{
	IF_F(!this->_DataBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	return true;
}

bool _filterRotate::start(void)
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

void _filterRotate::update(void)
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

		Point2f pCenter = Point2f(mIn.cols / 2, mIn.rows / 2);
		Size s = mIn.size();

		for (int j = 0; j < m_nProduce; j++)
		{
			Mat mNoise = Mat::zeros(s, mIn.type());
			gen.fill(mNoise, m_noiseType,
					cv::Scalar(m_noiseMean, m_noiseMean, m_noiseMean),
					cv::Scalar(m_noiseDev, m_noiseDev, m_noiseDev));

			int rAngle = rand() % 358 + 1;
			Mat mRot = getRotationMatrix2D(pCenter, rAngle, 1);

			Mat mMask = Mat::zeros(s, CV_8UC1);
			cv::warpAffine(Mat(s, CV_8UC1, 255), mMask, mRot, s, INTER_LINEAR, BORDER_CONSTANT);

			Mat mOut = Mat::zeros(s, mIn.type());
			cv::warpAffine(mIn, mOut, mRot, s, INTER_LINEAR, BORDER_CONSTANT);

			mOut.copyTo(mNoise, mMask);
			mOut = mNoise;

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

