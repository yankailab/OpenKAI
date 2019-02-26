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

	m_dir = "/home/";
	m_subDir = "";
}

_HiphenIMG::~_HiphenIMG()
{
	close();
}

bool _HiphenIMG::init(void* pKiss)
{
	IF_F(!this->_TCPclient::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK,dir);
	KISSm(pK,subDir);

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
		m_iB += nR;

		if (m_nB > 0)
		{
			if(m_iB >= m_nB)
			{
				close();
				decodeData();
				m_bComplete = true;
				return;
			}
		}
		else
		{
			if(m_iB >= N_HIPHEN_HEADER)
			{
				decodeHeader();
				m_nB = N_HIPHEN_HEADER + m_nbImg;
			}
		}

		if(nR == 0)
		{
			m_bComplete = true;
			return;
		}
	}
}

void _HiphenIMG::decodeHeader(void)
{
	int i;
	char pB[129];

	pB[N_HIPHEN_FILENAME] = 0;
	memcpy(pB, m_pBuf, N_HIPHEN_FILENAME);
	for(i=0;i<128;i++)
	{
		IF_CONT(pB[i] != ' ');
		pB[i]=0;
		break;
	}
	m_fileName = string(pB);

	pB[N_HIPHEN_FILESIZE]=0;
	memcpy(pB, &m_pBuf[N_HIPHEN_FILENAME], N_HIPHEN_FILESIZE);
	m_nbImg = atoi(pB);
}

void _HiphenIMG::decodeData(void)
{
	if(m_subDir.empty())
		m_subDir = m_dir + tFormat() + "/";
	else
		m_subDir = m_dir + m_subDir;

	string cmd = "mkdir " + m_subDir;
	system(cmd.c_str());

	IF_(m_fileName.length()<=0);
	IF_(m_nbImg <= 0);

	m_fileName = m_subDir + m_fileName;
	ofstream of(m_fileName.c_str(), ofstream::binary);
	IF_(!of.is_open());
	of.write((char*)&m_pBuf[N_HIPHEN_HEADER], m_nbImg);

	of.flush();
	of.close();

	//TODO: add exif GPS data
}

}
