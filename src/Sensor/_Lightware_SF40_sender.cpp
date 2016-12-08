/*
 *  Created on: Nov 4, 2016
 *      Author: yankai
 */
#include "_Lightware_SF40_sender.h"

namespace kai
{
_Lightware_SF40_sender::_Lightware_SF40_sender()
{
	_ThreadBase();

	m_pSerialPort = NULL;
	m_dAngle = 1;
}

_Lightware_SF40_sender::~_Lightware_SF40_sender()
{
}

bool _Lightware_SF40_sender::init(void* pKiss)
{
	CHECK_F(!this->_ThreadBase::init(pKiss));

	return true;
}

bool _Lightware_SF40_sender::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		LOG(ERROR)<< retCode;
		m_bThreadON = false;
		return false;
	}

	return true;
}

bool _Lightware_SF40_sender::link(void)
{
	return true;
}

void _Lightware_SF40_sender::update(void)
{
	while (m_bThreadON)
	{
		if (m_pSerialPort==NULL)continue;
		if (!m_pSerialPort->isOpen())continue;

		//Regular update loop
		this->autoFPSfrom();
		LD();
		this->autoFPSto();
	}

}

void _Lightware_SF40_sender::LD(void)
{
	NULL_(m_pSerialPort);
	CHECK_(!m_pSerialPort->isOpen());

	//?LD,aaa.a<CR><LF> <space>dd.dd<CR><LF>

	char str[128];

	for(double angle=0; angle<DEG_AROUND; angle+=m_dAngle)
	{
		sprintf(str, "?LD,%.1f\x0d\x0a", angle);
		m_pSerialPort->write((uint8_t*)str, strlen(str));
	}

}

void _Lightware_SF40_sender::Map(void)
{
	string TM = "?TM,360,0\x0d\x0a";

	m_pSerialPort->write((uint8_t*)TM.c_str(), TM.length());
}


bool _Lightware_SF40_sender::draw(Frame* pFrame, vInt4* pTextPos)
{
	return true;
}


}
