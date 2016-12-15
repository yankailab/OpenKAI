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

	LOG_E("TCP mode unknown");
	return false;
}

bool TCP::open(void)
{
	CHECK_T(m_status == opening);

	if(m_pServer)
	{
		//server mode
		CHECK_F(!m_pServer->start());
		m_status = opening;
		LOG_I("Server is opening");
		return true;
	}
	else if(m_pSocket)
	{
		//client mode
		CHECK_F(!m_pSocket->start());
		m_status = opening;
		LOG_I("Socket is opening");
	    return true;
	}

	return false;
}

void TCP::close(void)
{
	m_status = closed;
	if(m_pServer)m_pServer->complete();
	else if(m_pSocket)m_pSocket->complete();
	LOG_I("Closed");
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

	LOG_E("IO is NULL in read");
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

	LOG_E("IO is NULL in write");
	return false;
}

bool TCP::writeLine(uint8_t* pBuf, int nByte)
{
	CHECK_F(!write(pBuf, nByte));

	const char crlf[] = "\x0d\x0a";
	return write((uint8_t*)crlf, 2);
}

bool TCP::draw(void)
{
	if(m_pServer)
	{
		m_pServer->draw();
	}
	else if(m_pSocket)
	{
		m_pSocket->draw();
	}

	return true;
}


}
