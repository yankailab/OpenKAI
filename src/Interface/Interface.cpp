/*
 * Interface.cpp
 *
 *  Created on: Dec 7, 2016
 *      Author: root
 */

#include "Interface.h"

namespace kai
{

_Interface::_Interface()
{
	m_type = none;
	m_status = unknown;

	m_pSerialPort = NULL;
	m_pSportName = NULL;
	m_baudRate = 115200;

	m_pServer = NULL;
	m_pPeer = NULL;
	m_pMsg = NULL;

	m_pFile = NULL;
	m_pFileName = NULL;

	m_nBuf = 0;
	m_pBuf = NULL;
	m_strBuf = "";
}

_Interface::~_Interface()
{
}

bool _Interface::init(void* pKiss)
{
	CHECK_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	string param;

	//type
	param = "";
	F_ERROR_F(pK->v("type", &param));
	if (param == "uart")
		m_type = uart;
	else if (param == "file")
		m_type = file;
	else if (param == "network")
		m_type = network;
	else
		m_type = none;

	//source dependent
	if (m_type == uart)
	{
		m_pSportName = new string();
		F_ERROR_F(pK->v("portName", m_pSportName));
		F_ERROR_F(pK->v("baudrate", &m_baudRate));
		m_pSerialPort = new SerialPort();
	}
	else if (m_type == file)
	{
		m_pFileName = new string();
		F_ERROR_F(pK->v("fileName", m_pFileName));
		m_pFile = new FileIO();
		F_ERROR_F(m_pFile->open(m_pFileName));
	}
	else if (m_type == network)
	{
	}

	return true;
}

bool _Interface::link(void)
{
	NULL_F(m_pKiss);
	Kiss* pK = (Kiss*) m_pKiss;

	string param = "";

	if (m_type == uart)
	{
	}
	else if (m_type == file)
	{
	}
	else if (m_type == network)
	{
		F_INFO(pK->v("_server", &param));
		m_pServer = (_server*) (pK->root()->getChildInstByName(&param));
		F_INFO(pK->v("_peer", &param));
		m_pPeer = (_peer*) (pK->root()->getChildInstByName(&param));
	}

	return true;
}

bool _Interface::start(void)
{
	//No thread for file

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

void _Interface::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();


		this->autoFPSto();
	}

}

} /* namespace kai */
