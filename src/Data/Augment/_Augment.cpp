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
	m_dRot = 0;
	m_nRot = 0;
	m_bRot = false;
	m_bFlip = false;
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

	KISSm(pK, bFlip);
	KISSm(pK, bRot);
	KISSm(pK, dRot);
	KISSm(pK, nRot);
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

		//TODO: Random rotate angles


		for(int j=1; j<m_nRot; j++)
		{
			Mat mRot = getRotationMatrix2D(pCenter, m_dRot*j, 1);
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

		Point2f pCenter = Point2f(mIn.cols/2,mIn.rows/2);
		Size s = Size(mIn.cols, mIn.rows);

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

