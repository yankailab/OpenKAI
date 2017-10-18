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
	m_scaleFrom = 0.5;
	m_scaleTo = 1.5;
	m_nScaling = 10;
	m_bDeleteOriginal = false;

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
	KISSm(pK, scaleFrom);
	KISSm(pK, scaleTo);
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

	exit(0);
}

void _Augment::rotate(void)
{
	IF_(!m_bRot);
	IF_(getDirFileList()<=0);
	IF_(!openOutput());

	int i;
	int nTot = 0;
	Mat mOut;
	string fName;

	for(i=0; i<m_vFileIn.size(); i++)
	{
		string fNameIn = m_dirIn + m_vFileIn[i];
		Mat mIn = cv::imread(fNameIn.c_str());
		IF_CONT(mIn.empty());

		fName = m_dirOut + uuid() + m_extOut;
		cv::imwrite(fName, mIn, m_PNGcompress);

		Point2f pCenter = Point2f(mIn.cols/2,mIn.rows/2);
		Size s = Size(mIn.cols, mIn.rows);

		for(int j=0; j<m_nRot; j++)
		{
			Mat mRot = getRotationMatrix2D(pCenter, rand()%358+1, 1);
			cv::warpAffine(mIn, mOut, mRot, s, INTER_LINEAR, BORDER_CONSTANT);

			fName = m_dirOut + uuid() + m_extOut;
			cv::imwrite(fName, mOut, m_PNGcompress);

			LOG_I("Rot: " << nTot << " " << fName);
		}

		if(m_bDeleteOriginal)
		{
			remove(fNameIn.c_str());
		}

		nTot++;
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

	int i;
	int nTot = 0;
	Mat mOut;
	string fName;

	double dScaling = abs(m_scaleTo - m_scaleFrom);

	for(i=0; i<m_vFileIn.size(); i++)
	{
		string fNameIn = m_dirIn + m_vFileIn[i];
		Mat mIn = cv::imread(fNameIn.c_str());
		IF_CONT(mIn.empty());

		fName = m_dirOut + uuid() + m_extOut;
		cv::imwrite(fName, mIn, m_PNGcompress);

		for(int j=0; j<m_nScaling; j++)
		{
			double rScale = dScaling * (((double)rand()) / ((double)RAND_MAX));
			cv::resize(mIn, mOut, cv::Size(), m_scaleTo+rScale, m_scaleTo+rScale);

			fName = m_dirOut + uuid() + m_extOut;
			cv::imwrite(fName, mOut, m_PNGcompress);

			LOG_I("Scaling: " << nTot << " " << fName);
		}

		if(m_bDeleteOriginal)
		{
			remove(fNameIn.c_str());
		}

		nTot++;
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

	int i;
	int nTot = 0;
	Mat mOut;
	string fName;

	for(i=0; i<m_vFileIn.size(); i++)
	{
		string fNameIn = m_dirIn + m_vFileIn[i];
		Mat mIn = cv::imread(fNameIn.c_str());
		IF_CONT(mIn.empty());

		fName = m_dirOut + uuid() + m_extOut;
		cv::imwrite(fName, mIn, m_PNGcompress);

		for(int j=-1; j<2; j++)
		{
			cv::flip(mIn, mOut, j);

			fName = m_dirOut + uuid() + m_extOut;
			cv::imwrite(fName, mOut, m_PNGcompress);

			LOG_I("Flip: " << nTot << " " << fName);
		}

		if(m_bDeleteOriginal)
		{
			remove(fNameIn.c_str());
		}

		nTot++;
	}

	LOG_I("Total flipped: " << nTot);
}

void _Augment::tone(void)
{

}

}

