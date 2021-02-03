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
	m_pGPS = NULL;
	m_iB = 0;
	m_nB = 0;
	m_nbImg = 0;
	m_dir = "";
	m_fileName = "";
}

_HiphenIMG::~_HiphenIMG()
{
	close();
}

bool _HiphenIMG::init(void* pKiss)
{
	IF_F(!this->_TCPclient::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	string n;

	n = "";
	F_ERROR_F(pK->v("_GPS", &n));
	m_pGPS = (_GPS*) (pK->getInst(n));
	IF_Fl(!m_pGPS, n + " not found");

	return true;
}

bool _HiphenIMG::start(void)
{
    IF_F(check()<0);
	return m_pT->start(getUpdate, this);
}

void _HiphenIMG::update(void)
{
	while(m_pT->bRun())
	{
		int nR = ::recv(m_socket, &m_pBuf[m_iB], N_HIPHEN_BUF - m_iB, 0);
		m_iB += nR;

		if(m_nB <= 0 && m_iB >= N_HIPHEN_HEADER)
		{
			decodeHeader();
			m_nB = N_HIPHEN_HEADER + m_nbImg;
		}

		if(m_nB > 0 && m_iB >= m_nB)
		{
			close();
			decodeData();
			m_bComplete = true;
			return;
		}

		if(nR == 0)
		{
			close();
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
	IF_(m_fileName.length()<=0);
	IF_(m_nbImg <= 0);

	m_fileName = m_dir + m_fileName;
	ofstream of(m_fileName.c_str(), ofstream::binary);
	if(!of.is_open())
	{
		LOG_E("File open failed: " + m_fileName);
		return;
	}

	of.write((char*)&m_pBuf[N_HIPHEN_HEADER], m_nbImg);
	of.close();

	if(m_pGPS)
	{
		LL_POS pLL = m_pGPS->getLLpos();
		string lat = lf2str(pLL.m_lat, 7);
		string lon = lf2str(pLL.m_lng, 7);
		string alt = lf2str(pLL.m_altRel, 3);
		string cmd = "exiftool -overwrite_original -GPSLongitude=\"" + lon + "\" -GPSLatitude=\"" + lat + "\" " + m_fileName;
		system(cmd.c_str());
	}
}

}
