/*
 * _Raspivid.cpp
 *
 *  Created on: May 6, 2018
 *      Author: yankai
 */

#include "_Raspivid.h"

namespace kai
{

	_Raspivid::_Raspivid()
	{
		m_type = vision_raspivid;
		m_cMode = raspivid_rgb;
		m_cmd = "raspividyuv";
		m_option = "-n -rot 180";
		m_pFr = NULL;
		m_iFr = -1;
		m_nFB = 0;
		m_pFB = NULL;
	}

	_Raspivid::~_Raspivid()
	{
		m_nFB = 0;
		DEL(m_pFB);

		m_bOpen = false;

		m_iFr = -1;
		if (m_pFr)
		{
			pclose(m_pFr);
			m_pFr = NULL;
		}
	}

	bool _Raspivid::init(void *pKiss)
	{
		IF_F(!_VisionBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("cmd", &m_cmd);
		pK->v("option", &m_option);

		string cMode = "rgb";
		F_INFO(pK->v("cMode", &cMode));

		if (cMode == "rgb")
		{
			m_cMode = raspivid_rgb;
			m_nFB = m_vSize.x * m_vSize.y * 3;
		}
		else
		{
			//round up resolution for Raspividyuv output
			//https://picamera.readthedocs.io/en/latest/recipes2.html#unencoded-image-capture-yuv-format
			int rx = (m_vSize.x + 31) / 32 * 32;
			int ry = (m_vSize.y + 15) / 16 * 16;

			m_cMode = raspivid_y;
			m_nFB = rx * ry;
		}

		m_pFB = new uint8_t[m_nFB];

		return true;
	}

	bool _Raspivid::open(void)
	{
		string cmdR = m_cmd + " " + "-w " + i2str(m_vSize.x) + " " + "-h " + i2str(m_vSize.y) + " -fps " + i2str((int)m_pT->getTargetFPS()) + " -t 0 -o - " + m_option + " ";

		if (m_cMode == raspivid_rgb)
		{
			cmdR += "-rgb";
		}
		else
		{
			cmdR += "--luma";
		}

		m_pFr = popen(cmdR.c_str(), "r");
		if (m_pFr <= 0)
		{
			LOG_E("popen failed: " + m_cmd);
			return false;
		}
		m_iFr = fileno(m_pFr);

		m_bOpen = true;
		return true;
	}

	bool _Raspivid::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	void _Raspivid::update(void)
	{
		while (m_pT->bRun())
		{
			if (!m_bOpen)
			{
				if (!open())
				{
					m_pT->sleepT(SEC_2_USEC);
					continue;
				}
			}

			m_pT->autoFPSfrom();

			int nR = 0;
			while ((nR += read(m_iFr, &m_pFB[nR], m_nFB - nR)) < m_nFB)
				;

			if (m_cMode == raspivid_y)
			{
				m_fBGR.copy(Mat(m_vSize.y, m_vSize.x, CV_8UC1, m_pFB));
			}
			else
			{
				Frame fRGB;
				fRGB = Mat(m_vSize.y, m_vSize.x, CV_8UC3, m_pFB);
				//m_fBGR.copy(fRGB.cvtColor(COLOR_RGB2BGR));
				m_fBGR = fRGB.cvtColor(COLOR_RGB2BGR);
			}

			m_pT->autoFPSto();
		}
	}

}
