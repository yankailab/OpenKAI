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

	int _ImgFile::init(const json& j)
	{
		CHECK_(_VisionBase::init(j));

		= j.value("file", &m_file);

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

		m_fRGB.copy(m);
		m_vSizeRGB.x = m.cols;
		m_vSizeRGB.y = m.rows;

		m_bOpen = true;
		return true;
	}

	int _ImgFile::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	void _ImgFile::update(void)
	{
		while (m_pT->bAlive())
		{
			if (!m_bOpen)
			{
				if (!open())
				{
					m_pT->sleepT(SEC_2_USEC);
					continue;
				}
			}

			m_pT->autoFPS();

		}
	}

}
