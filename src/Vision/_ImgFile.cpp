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
	}

	_ImgFile::~_ImgFile()
	{
	}

	bool _ImgFile::init(const json &j)
	{
		IF_F(!_VisionBase::init(j));

		jKv(j, "file", m_file);

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

		{
			std::lock_guard<std::mutex> lock(m_mutexRGB);
			m.copyTo(m_mRGB);
		}
		m_vSizeRGB.x() = m.cols;
		m_vSizeRGB.y() = m.rows;

		m_bOpened = true;
		return true;
	}

	bool _ImgFile::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	void _ImgFile::update(void)
	{
		while (m_pT->bRun())
		{
			if (!m_bOpened)
			{
				if (!open())
				{
					m_pT->sleepT(NSEC_SEC);
					continue;
				}
			}

			m_pT->autoFPS();
		}
	}

}
