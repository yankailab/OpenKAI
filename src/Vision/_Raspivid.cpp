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
	m_cmd = "";
	m_option = "";
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
	m_vImg.clear();

	m_iFr = -1;
	if (m_pFr)
	{
		pclose(m_pFr);
		m_pFr = NULL;
	}
}

bool _Raspivid::init(void* pKiss)
{
	IF_F(!_VisionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK, cmd);
	KISSm(pK, option);

	//round up resolution for Raspividyuv output
	//https://picamera.readthedocs.io/en/latest/recipes2.html#unencoded-image-capture-yuv-format
	m_w = (m_w + 31) / 32 * 32;
	m_h = (m_h + 15) / 16 * 16;

	string cMode = "rgb";
	F_INFO(pK->v("cMode", &cMode));

	if (cMode == "rgb")
	{
		m_cMode = raspivid_rgb;
		m_nFB = m_w * m_h * 3;
	}
	else
	{
		m_cMode = raspivid_y;
		m_nFB = m_w * m_h;
	}

	m_pFB = new uint8_t[m_nFB];

	return true;
}

bool _Raspivid::open(void)
{
	string cmdR = m_cmd + " " + "-w " + i2str(m_w) + " " + "-h " + i2str(m_h)
			+ " " + "-fps " + i2str(m_targetFPS) + " " + "-t 0 " + "-o - "
			+ m_option + " ";

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
	m_vImg.clear();

	m_bOpen = true;
	return true;
}

bool _Raspivid::start(void)
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

void _Raspivid::update(void)
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

		while (read(m_iFr, m_pFB, m_nFB) != m_nFB);

		if (m_cMode == raspivid_y)
		{
			m_fBGR.copy(Mat(m_h, m_w, CV_8UC1, m_pFB));
		}
		else
		{
			Frame fRGB;
			fRGB = Mat(m_h, m_w, CV_8UC3, m_pFB);
			m_fBGR.copy(fRGB.cvtColor(COLOR_RGB2BGR));
		}

		this->autoFPSto();
	}
}

}
