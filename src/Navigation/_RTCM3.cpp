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
	IF_F(!this->_ModuleBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	string n = "";
	F_ERROR_F(pK->v("_IOBase", &n));
	m_pIO = (_IOBase*) (pK->getInst(n));
	IF_Fl(!m_pIO, "_IOBase not found");

	return true;
}

bool _RTCM3::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdate, this);
	if (retCode != 0)
	{
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _RTCM3::update(void)
{
	while(m_pT->bRun())
	{
		m_pT->autoFPSfrom();

		decode();
		m_msg = "";

		m_pT->autoFPSto();
	}
}

void _RTCM3::decode(void)
{
}

void _RTCM3::draw(void)
{
	this->_ModuleBase::draw();

	if(!m_pIO->isOpen())
	{
		addMsg("Not connected");
		return;
	}

}

}
