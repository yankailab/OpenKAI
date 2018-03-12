/*
 * _imgRotate.cpp
 *
 *  Created on: Mar 12, 2018
 *      Author: yankai
 */

#include "_imgRotate.h"

namespace kai
{

_imgRotate::_imgRotate()
{
	m_bgNoiseMean = 0;
	m_bgNoiseDev = 0;
	m_bgNoiseType = cv::RNG::NORMAL;

	m_nProduce = 1;
	m_bDeleteOriginal = false;
	m_bSaveOriginalCopy = false;
	m_progress = 0.0;
}

_imgRotate::~_imgRotate()
{
	reset();
}

bool _imgRotate::init(void* pKiss)
{
	IF_F(!this->_DataBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	KISSm(pK, bgNoiseMean);
	KISSm(pK, bgNoiseDev);
	KISSm(pK, bgNoiseType);

	KISSm(pK, nProduce);
	KISSm(pK, bDeleteOriginal);
	KISSm(pK, bSaveOriginalCopy);

	IF_F(getDirFileList() <= 0);
	return true;
}

bool _imgRotate::link(void)
{
	IF_F(!this->_DataBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	return true;
}

bool _imgRotate::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _imgRotate::update(void)
{
	srand(time(NULL));

	process();
}

void _imgRotate::process(void)
{
	cv::RNG gen(cv::getTickCount());

	int nTot = 0;
	m_progress = 0.0;

	for (int i = 0; i < m_vFileIn.size(); i++)
	{
		string fNameIn = m_vFileIn[i];
		string dirNameIn = getFileDir(fNameIn);
		Mat mIn = cv::imread(fNameIn.c_str());
		IF_CONT(mIn.empty());

		if(m_bSaveOriginalCopy)
			cv::imwrite(dirNameIn + uuid() + m_extOut, mIn, m_PNGcompress);
		if (m_bDeleteOriginal)
			remove(fNameIn.c_str());

		Point2f pCenter = Point2f(mIn.cols / 2, mIn.rows / 2);
		Size s = mIn.size();

		for (int j = 0; j < m_nProduce; j++)
		{
			Mat mNoise = Mat::zeros(s, mIn.type());
			gen.fill(mNoise, m_bgNoiseType,
					cv::Scalar(m_bgNoiseMean, m_bgNoiseMean, m_bgNoiseMean),
					cv::Scalar(m_bgNoiseDev, m_bgNoiseDev, m_bgNoiseDev));

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
			LOG_I("Progress: " << (int)(m_progress * 100) << "%");
		}
	}

	LOG_I("------ Complete: Total produced: " << nTot);
}

}

