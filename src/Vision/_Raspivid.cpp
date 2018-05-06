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
	m_cmdR = "";
	m_pFr = NULL;
	m_iFr = -1;
	m_cmdW = "";
	m_pFw = NULL;
	m_iFw = -1;
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
	KISSm(pK, cmdW);

	return true;
}

void _Raspivid::reset(void)
{
	this->_VisionBase::reset();

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
	m_pFr = popen(m_cmdR.c_str(), "r");
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
	bool bSkip = true;
	bool bImgReady = false;
	bool bFF = false;
	int nB = -1;
	uint8_t pB[RASPIVID_N_BUF];

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

		nB = read(m_iFr, pB, RASPIVID_N_BUF);

		for (int i = 0; i < nB; i++)
		{
			uint8_t c = pB[i];

			if (bFF && c == (uint8_t) 0xd8)
			{
				bSkip = false;
				m_vImg.push_back((uint8_t) 0xff);
			}

			if (bFF && c == 0xd9)
			{
				bImgReady = true;
				m_vImg.push_back((uint8_t) 0xd9);
				bSkip = true;
			}

			bFF = c == 0xff;
			if (!bSkip)
			{
				m_vImg.push_back(c);
			}

			IF_CONT(!bImgReady);

			if (m_vImg.size() != 0)
			{
				cv::Mat vM(m_vImg);
				m_fBGR = imdecode(vM, 1);

				Size s = m_fBGR.size();
				m_w = s.width;
				m_h = s.height;
				m_cW = m_w / 2;
				m_cH = m_h / 2;

				m_pTPP->wakeUp();
			}

			bImgReady = false;
			bSkip = true;
			m_vImg.clear();
		}

		if(m_iFw > 0)
		{
			write(m_iFw, pB, nB);
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
