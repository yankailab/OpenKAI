/*
 * _HiphenIMG.cpp
 *
 *  	Created on: Feb 23, 2019
 *      Author: yankai
 */

#include "_HiphenIMG.h"

namespace kai
{

_HiphenIMG::_HiphenIMG()
{
	m_iB = 0;
	m_nB = 0;
	m_nbImg = 0;
	m_bComplete = false;
}

_HiphenIMG::~_HiphenIMG()
{
	close();
}

bool _HiphenIMG::init(void* pKiss)
{
	IF_F(!this->_TCPclient::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	return true;
}

bool _HiphenIMG::start(void)
{
	int retCode;
	if(!m_bRThreadON)
	{
		m_bRThreadON = true;
		retCode = pthread_create(&m_rThreadID, 0, getUpdateThreadR, this);
		if (retCode != 0)
		{
			LOG_E(retCode);
			m_bRThreadON = false;
			return false;
		}
	}

	return true;
}

void _HiphenIMG::updateR(void)
{
	while (m_bRThreadON)
	{
		int nR = ::recv(m_socket, &m_pBuf[m_iB], N_HIPHEN_BUF - m_iB, 0);
		if (nR <= 0)
		{
			close();
			decodeData();
			m_bComplete = true;
			return;
		}

		m_iB += nR;

		if(m_nB == 0)
		{
			if(m_iB >= N_HIPHEN_HEADER)
			{
				m_nbImg = atoi((char*)&m_pBuf[128]);
				m_nB = N_HIPHEN_HEADER + m_nbImg;
			}
		}
		else if (m_iB >= m_nB)
		{
			close();
			decodeData();
			m_bComplete = true;
			return;
		}
	}
}

void _HiphenIMG::decodeData(void)
{
	if(m_iB < m_nB)
	{
		LOG_E("Incomplete data");
		return;
	}

	string fileName((char*)m_pBuf);
	fstream fs;
	fs.open(fileName.c_str(), ios::out|ios::binary);
	IF_(!fs.is_open());
	IF_(!fs.write((char*)m_pBuf[N_HIPHEN_HEADER], m_nbImg));

	fs.flush();
	fs.close();

	//TODO: add exif GPS data
}

bool _HiphenIMG::bComplete(void)
{
	return m_bComplete;
}

}
