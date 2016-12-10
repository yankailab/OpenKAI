#include "TCP.h"
#include "../Script/Kiss.h"

namespace kai
{

TCP::TCP(void)
{
	IO();
	m_type = tcp;
	m_pSocket = NULL;
	m_pServer = NULL;
}

TCP::~TCP(void)
{
	close();
	DEL(m_pServer);
	DEL(m_pSocket);
}

bool TCP::init(void* pKiss)
{
	CHECK_F(!this->IO::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	Kiss* pCC;

	pCC = pK->o("server");
	if(!pCC->empty())
	{
		//server mode
		m_pServer = new _server();
		F_ERROR_F(m_pServer->init(pCC));
		m_status = closed;
		return true;
	}

	pCC = pK->o("client");
	if(!pCC->empty())
	{
		//client mode
		m_pSocket = new _socket();
		F_ERROR_F(m_pSocket->init(pCC));
		m_status = closed;
		return true;
	}

	LOG(ERROR)<<"TCP mode unknown";
	return false;
}

bool TCP::open(void)
{
	CHECK_T(m_status == opening);

	if(m_pServer)
	{
		//server mode
		CHECK_F(m_pServer->start());
		m_status = opening;
		return true;
	}
	else if(m_pSocket)
	{
		//client mode
		CHECK_F(m_pSocket->start());
		m_status = opening;
	    return true;
	}

	return false;
}

void TCP::close(void)
{
	m_status = closed;
	if(m_pServer)m_pServer->complete();
	else if(m_pSocket)m_pSocket->complete();
}

int TCP::read(uint8_t* pBuf, int nByte)
{
	if(m_status != opening)return -1;

	if(m_pServer)
	{
		//server mode
		_socket* pSocket = m_pServer->getFirstSocket();
		if(!pSocket)return -1;

	    return pSocket->read(pBuf,nByte);
	}
	else if(m_pSocket)
	{
		//client mode
	    return m_pSocket->read(pBuf,nByte);
	}

	return -1;
}

bool TCP::write(uint8_t* pBuf, int nByte)
{
	CHECK_F(m_status != opening);

	if(m_pServer)
	{
		//server mode
		_socket* pSocket = m_pServer->getFirstSocket();
		if(!pSocket)return -1;

	    return pSocket->write(pBuf,nByte);
	}
	else if(m_pSocket)
	{
		//client mode
	    return m_pSocket->write(pBuf,nByte);
	}

	return false;
}

bool TCP::writeLine(uint8_t* pBuf, int nByte)
{
	CHECK_F(!write(pBuf, nByte));

	const char crlf[] = "\x0d\x0a";
	return write((uint8_t*)crlf, 2);
}

bool TCP::draw(Frame* pFrame, vInt4* pTextPos)
{
	CHECK_F(!this->BASE::draw(pFrame,pTextPos));

	if(m_pServer)
	{
		m_pServer->draw(pFrame,pTextPos);
	}
	else if(m_pSocket)
	{
		m_pSocket->draw(pFrame,pTextPos);
	}

	return true;
}


}
