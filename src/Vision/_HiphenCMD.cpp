/*
 * _HiphenCMD.cpp
 *
 *  Created on: Aug 22, 2015
 *      Author: yankai
 */

#include "_HiphenCMD.h"

namespace kai
{

_HiphenCMD::_HiphenCMD()
{
	m_nSave = 1;
	m_nSnap = 3;
}

_HiphenCMD::~_HiphenCMD()
{
}

bool _HiphenCMD::init(void* pKiss)
{
	IF_F(!_TCPclient::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK, nSave);
	KISSm(pK, nSnap);

	return true;
}

bool _HiphenCMD::start(void)
{
	int retCode;

	if(!m_bThreadON)
	{
		m_bThreadON = true;
		retCode = pthread_create(&m_threadID, 0, getUpdateThreadW, this);
		if (retCode != 0)
		{
			LOG_E(retCode);
			m_bThreadON = false;
			return false;
		}
	}

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

void _HiphenCMD::updateW(void)
{
	while (m_bThreadON)
	{
		if (!isOpen())
		{
			if (!open())
			{
				this->sleepTime(USEC_1SEC);
				continue;
			}
		}

		this->autoFPSfrom();

		string cmd = "{\"command\":\"get_status\"}";
		m_fifoW.input((uint8_t*)cmd.c_str(), cmd.length());


		uint8_t pB[N_IO_BUF];
		int nB;
		while((nB = m_fifoW.output(pB, N_IO_BUF)) > 0)
		{
			int nSend = ::send(m_socket, pB, nB, 0);
			if (nSend == -1)
			{
				if(errno == EAGAIN)break;
				if(errno == EWOULDBLOCK)break;
				LOG_E("send error: " + i2str(errno));
				close();
				break;
			}

			LOG_I("send: " + i2str(nSend) + " bytes");
		}

		this->autoFPSto();
	}
}

void _HiphenCMD::updateR(void)
{
	while (m_bRThreadON)
	{
		if (!isOpen())
		{
			::sleep(1);
			continue;
		}

		//blocking mode, no FPS control
		uint8_t pB[N_IO_BUF];
		int nR = ::recv(m_socket, pB, N_IO_BUF, 0);
		if (nR <= 0)
		{
			LOG_E("recv error: " + i2str(errno));
			close();
			continue;
		}

//		m_fifoR.input(pB,nR);
		string strR((char*)pB);
		LOG_I("Received: " + strR);

		this->wakeUpLinked();
	}
}

}
