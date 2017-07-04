/*
 * UDP.cpp
 *
 *  Created on: June 16, 2016
 *      Author: yankai
 */

#include "UDP.h"

namespace kai
{

UDP::UDP()
{
	m_pSender = NULL;
	m_pReceiver = NULL;
}

UDP::~UDP()
{
}

bool UDP::init(void* pKiss)
{
	IF_F(!this->_IOBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	return true;
}

bool UDP::link(void)
{
	IF_F(!this->_IOBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	string iName;

	iName = "";
	F_ERROR_F(pK->v("_UDPclient", &iName));
	m_pSender = (_UDPclient*) (pK->root()->getChildInstByName(&iName));

	iName = "";
	F_ERROR_F(pK->v("_UDPserver", &iName));
	m_pReceiver = (_UDPserver*) (pK->root()->getChildInstByName(&iName));

	m_pSender->m_bSendOnly = true;
	m_pReceiver->m_bReceiveOnly = true;

	return true;
}

bool UDP::write(uint8_t* pBuf, int nB)
{
	return m_pSender->write(pBuf,nB);
}

bool UDP::writeLine(uint8_t* pBuf, int nB)
{
	const char pCRLF[] = "\x0d\x0a";

	IF_F(!write(pBuf, nB));
	return write((unsigned char*)pCRLF, 2);
}

int UDP::read(uint8_t* pBuf, int nB)
{
	return m_pReceiver->read(pBuf, nB);
}

void UDP::close(void)
{
	IF_(m_ioStatus!=io_opened);

	m_pSender->close();
	m_pReceiver->close();

	this->_IOBase::close();
}

bool UDP::draw(void)
{
	IF_F(!this->_ThreadBase::draw());
	Window* pWin = (Window*)this->m_pWindow;
	Mat* pMat = pWin->getFrame()->getCMat();

	string msg;
	pWin->addMsg(&msg);

	return true;
}

}
