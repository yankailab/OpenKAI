/*
 * _RTCM3.cpp
 *
 *  Created on: Jun 3, 2020
 *      Author: yankai
 */

#include "_RTCM3.h"

namespace kai
{

_RTCM3::_RTCM3()
{
	m_pIO = NULL;
	m_nRead = 0;
	m_iRead = 0;
	m_msg = "";

}

_RTCM3::~_RTCM3()
{
}

bool _RTCM3::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	string iName = "";
	F_ERROR_F(pK->v("_IOBase", &iName));
	m_pIO = (_IOBase*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pIO, "_IOBase not found");

	return true;
}

bool _RTCM3::start(void)
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

void _RTCM3::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		decode();
		m_msg = "";

		this->autoFPSto();
	}
}

void _RTCM3::decode(void)
{
}

void _RTCM3::draw(void)
{
	this->_ThreadBase::draw();

	if(!m_pIO->isOpen())
	{
		addMsg("Not connected");
		return;
	}

}

}
