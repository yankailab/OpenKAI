/*
 * _ImgFile.cpp
 *
 *  Created on: Aug 5, 2018
 *      Author: yankai
 */

#include "_ImgFile.h"

namespace kai
{

_ImgFile::_ImgFile()
{
	m_type = vision_file;
	m_file = "";
}

_ImgFile::~_ImgFile()
{
}

bool _ImgFile::init(void* pKiss)
{
	IF_F(!_VisionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("file",&m_file);

	return true;
}

bool _ImgFile::open(void)
{
	Mat m = imread(m_file);
	if (m.empty())
	{
		LOG_E("Cannot open file: " + m_file);
		return false;
	}

	m_fBGR.copy(m);
	m_w = m.cols;
	m_h = m.rows;
	m_cW = m_w / 2;
	m_cH = m_h / 2;

	m_bOpen = true;
	return true;
}

bool _ImgFile::start(void)
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

void _ImgFile::update(void)
{
	while (m_bThreadON)
	{
		if (!m_bOpen)
		{
			if (!open())
			{
				this->sleepTime(USEC_1SEC);
				continue;
			}
		}

		this->autoFPSfrom();

		this->autoFPSto();
	}
}

}
