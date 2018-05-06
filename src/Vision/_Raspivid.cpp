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
	m_type = raspivid;
	m_nChannel = 3;
	m_mType = CV_8UC3;
	m_cmdR = "";
	m_option = "";
	m_pFr = NULL;
	m_iFr = -1;
	m_cmdW = "";
	m_pFw = NULL;
	m_iFw = -1;
	m_nFB = 0;
	m_pFB = NULL;
}

_Raspivid::~_Raspivid()
{
	reset();
}

bool _Raspivid::init(void* pKiss)
{
	IF_F(!_VisionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	KISSm(pK, cmdR);
	KISSm(pK, option);
	KISSm(pK, nChannel);
	KISSm(pK, cmdW);

	m_nFB = m_w * m_h * m_nChannel;
	m_pFB = new uint8_t[m_nFB];

	if(m_nChannel == 3)
		m_mType = CV_8UC3;
	else if(m_nChannel == 1)
		m_mType = CV_8UC1;

	return true;
}

void _Raspivid::reset(void)
{
	this->_VisionBase::reset();

	m_nFB = 0;
	DEL(m_pFB);

	m_bOpen = false;
	m_vImg.clear();

	m_iFr = -1;
	if(m_pFr)
	{
		pclose(m_pFr);
		m_pFr = NULL;
	}

	m_iFw = -1;
	if(m_pFw)
	{
		pclose(m_pFw);
		m_pFw = NULL;
	}

}

bool _Raspivid::link(void)
{
	IF_F(!this->_VisionBase::link());
	return true;
}

bool _Raspivid::open(void)
{
	string cmdR = m_cmdR + " "
			+ "-w " + i2str(m_w) + " "
			+ "-h " + i2str(m_h) + " "
			+ "-fps " + i2str(m_targetFPS) + " "
			+ "-t 0 "
			+ "-o - "
			+ m_option + " ";

	if(m_nChannel <= 1)
	{
		cmdR += "--luma";
	}

	m_pFr = popen(cmdR.c_str(), "r");
	if( m_pFr <= 0 )
	{
		LOG_E("popen failed: " << m_cmdR);
		return false;
	}
	m_iFr = fileno(m_pFr);
	m_vImg.clear();

	if(!m_cmdW.empty())
	{
		m_pFw = popen(m_cmdW.c_str(), "w");
		if( m_pFw <= 0 )
		{
			LOG_E("popen failed: " << m_cmdW);
			return false;
		}
		m_iFw = fileno(m_pFw);
	}

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

	retCode = pthread_create(&m_pTPP->m_threadID, 0, getTPP, this);
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

		fflush(m_pFr);

		int nB = read(m_iFr, m_pFB, m_nFB);
		if(nB == m_nFB)
		{
			Mat m = Mat(m_h, m_w, m_mType, m_pFB);

			if(m_nChannel==1)
			{
				m_fBGR = m;
			}
			else
			{
				m_fBGR = m_fBGR.f8UC3();
				cv::cvtColor(m_fBGR.m_mat, m_fBGR.m_mat, CV_YUV2BGR);
			}

			m_pTPP->wakeUp();

			if(m_iFw > 0)
				write(m_iFw, m_pFB, m_nFB);
		}

		this->autoFPSto();
	}
}

void _Raspivid::updateTPP(void)
{
	while (m_bThreadON)
	{
		m_pTPP->sleepTime(0);
		postProcess();
	}
}

}
