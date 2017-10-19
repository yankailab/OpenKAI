/*
 * _Augment.cpp
 *
 *  Created on: Oct 12, 2017
 *      Author: yankai
 */

#include "_Augment.h"

namespace kai
{

_Augment::_Augment()
{
	m_bRot = false;
	m_nRot = 0;
	m_bFlip = false;
	m_bScaling = false;
	m_dScaling = 0.25;
	m_nScaling = 10;
	m_bDeleteOriginal = false;
	m_progress = 0.0;
}

_Augment::~_Augment()
{
}

bool _Augment::init(void* pKiss)
{
	IF_F(!this->_DataBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	KISSm(pK, bRot);
	KISSm(pK, nRot);
	KISSm(pK, bFlip);
	KISSm(pK, bScaling);
	KISSm(pK, dScaling);
	KISSm(pK, nScaling);
	KISSm(pK, bDeleteOriginal);

	return true;
}

bool _Augment::link(void)
{
	IF_F(!this->_DataBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	return true;
}

bool _Augment::start(void)
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

void _Augment::update(void)
{
	srand(time(NULL));

	rotate();
	move();
	scaling();
	crop();
	flip();
	tone();
}

void _Augment::rotate(void)
{
	IF_(!m_bRot);
	IF_(getDirFileList()<=0);
	IF_(!openOutput());

	int nTot = 0;
	Mat mOut;
	m_progress = 0.0;

	for(int i=0; i<m_vFileIn.size(); i++)
	{
		string fNameIn = m_dirIn + m_vFileIn[i];
		Mat mIn = cv::imread(fNameIn.c_str());
		IF_CONT(mIn.empty());

		if(m_bDeleteOriginal)
		{
			cv::imwrite(m_dirOut + uuid() + m_extOut, mIn, m_PNGcompress);
			remove(fNameIn.c_str());
		}

		Point2f pCenter = Point2f(mIn.cols/2,mIn.rows/2);
		Size s = Size(mIn.cols, mIn.rows);

		for(int j=0; j<m_nRot; j++)
		{
			Mat mRot = getRotationMatrix2D(pCenter, rand()%358+1, 1);
			cv::warpAffine(mIn, mOut, mRot, s, INTER_LINEAR, BORDER_CONSTANT);

			cv::imwrite(m_dirOut + uuid() + m_extOut, mOut, m_PNGcompress);
		}

		nTot++;
		double prog = (double)nTot/(double)m_vFileIn.size();
		if(prog - m_progress > 0.1)
		{
			m_progress = prog;
			LOG_I("Rot: " << (int)(m_progress * 100) << "%");
		}
	}

	LOG_I("Total rotated: " << nTot);
}

void _Augment::move(void)
{

}

void _Augment::scaling(void)
{
	IF_(!m_bScaling);
	IF_(getDirFileList()<=0);
	IF_(!openOutput());

	int nTot = 0;
	Mat mOut;
	m_progress = 0.0;

	for(int i=0; i<m_vFileIn.size(); i++)
	{
		string fNameIn = m_dirIn + m_vFileIn[i];
		Mat mIn = cv::imread(fNameIn.c_str());
		IF_CONT(mIn.empty());

		if(m_bDeleteOriginal)
		{
			cv::imwrite(m_dirOut + uuid() + m_extOut, mIn, m_PNGcompress);
			remove(fNameIn.c_str());
		}

		for(int j=0; j<m_nScaling; j++)
		{
			double rScaleW = 1.0 - m_dScaling * NormRand();
			int dSizeW = ((double)mIn.cols) * rScaleW;
			int dRandW = ((double)mIn.cols - dSizeW) * NormRand();

			double rScaleH = 1.0 - m_dScaling * NormRand();
			int dSizeH = ((double)mIn.rows) * rScaleH;
			int dRandH = ((double)mIn.rows - dSizeH) * NormRand();

			mOut = mIn(cv::Rect(dRandW, dRandH, dSizeW, dSizeH));

			cv::imwrite(m_dirOut + uuid() + m_extOut, mOut, m_PNGcompress);
		}

		nTot++;
		double prog = (double)nTot/(double)m_vFileIn.size();
		if(prog - m_progress > 0.1)
		{
			m_progress = prog;
			LOG_I("Scaling: " << (int)(m_progress * 100) << "%");
		}
	}

	LOG_I("Total scaled: " << nTot);

}

void _Augment::crop(void)
{

}

void _Augment::flip(void)
{
	IF_(!m_bFlip);
	IF_(getDirFileList()<=0);
	IF_(!openOutput());

	int nTot = 0;
	Mat mOut;
	m_progress = 0.0;

	for(int i=0; i<m_vFileIn.size(); i++)
	{
		string fNameIn = m_dirIn + m_vFileIn[i];
		Mat mIn = cv::imread(fNameIn.c_str());
		IF_CONT(mIn.empty());

		if(m_bDeleteOriginal)
		{
			cv::imwrite(m_dirOut + uuid() + m_extOut, mIn, m_PNGcompress);
			remove(fNameIn.c_str());
		}

		Point2f pCenter = Point2f(mIn.cols/2,mIn.rows/2);
		Size s = Size(mIn.cols, mIn.rows);

		for(int j=-1; j<2; j++)
		{
			cv::flip(mIn, mOut, j);

			cv::imwrite(m_dirOut + uuid() + m_extOut, mOut, m_PNGcompress);
		}

		nTot++;
		double prog = (double)nTot/(double)m_vFileIn.size();
		if(prog - m_progress > 0.1)
		{
			m_progress = prog;
			LOG_I("Flip: " << (int)(m_progress * 100) << "%");
		}
	}

	LOG_I("Total fliped: " << nTot);
}

void _Augment::tone(void)
{

}

}

