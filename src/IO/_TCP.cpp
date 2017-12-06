#include "../Script/Kiss.h"
#include "_TCP.h"

namespace kai
{

_TCP::_TCP(void)
{
	m_ioType = io_tcp;
	m_pSocket = NULL;
	m_pServer = NULL;
}

_TCP::~_TCP(void)
{
	close();
	DEL(m_pServer);
	DEL(m_pSocket);
}

bool _TCP::init(void* pKiss)
{
	IF_F(!this->_IOBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	Kiss* pCC;

	pCC = pK->o("server");
	if(!pCC->empty())
	{
		//server mode
		m_pServer = new _TCPserver();
		F_ERROR_F(m_pServer->init(pCC));
		m_ioStatus = io_closed;
		return true;
	}

	pCC = pK->o("client");
	if(!pCC->empty())
	{
		//client mode
		m_pSocket = new _TCPsocket();
		F_ERROR_F(m_pSocket->init(pCC));
		m_ioStatus = io_closed;
		return true;
	}

	LOG_E("TCP mode unknown");
	return false;
}

bool _TCP::open(void)
{
	IF_T(m_ioStatus == io_opened);

	if(m_pServer)
	{
		//server mode
		IF_F(!m_pServer->start());
		m_ioStatus = io_opened;
		LOG_I("Server is opening");
		return true;
	}
	else if(m_pSocket)
	{
		//client mode
		IF_F(!m_pSocket->start());
		m_ioStatus = io_opened;
		LOG_I("Socket is opening");
	    return true;
	}

	return false;
}

void _TCP::close(void)
{
	m_ioStatus = io_closed;
	if(m_pServer)m_pServer->reset();
	else if(m_pSocket)m_pSocket->reset();
	LOG_I("Closed");
}

int _TCP::read(uint8_t* pBuf, int nByte)
{
	if(m_ioStatus != io_opened)return -1;

	if(m_pServer)
	{
		//server mode
		_TCPsocket* pSocket = m_pServer->getFirstSocket();
		if(!pSocket)return -1;

	    return pSocket->read(pBuf,nByte);
	}
	else if(m_pSocket)
	{
		//client mode
	    return m_pSocket->read(pBuf,nByte);
	}

	LOG_E("read: IO is NULL");
	return -1;
}

bool _TCP::write(uint8_t* pBuf, int nByte)
{
	IF_F(m_ioStatus != io_opened);

	if(m_pServer)
	{
		//server mode
		_TCPsocket* pSocket = m_pServer->getFirstSocket();
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

bool _TCP::writeLine(uint8_t* pBuf, int nByte)
{
	IF_F(!write(pBuf, nByte));

	const char crlf[] = "\x0d\x0a";
	return write((uint8_t*)crlf, 2);
}

bool _TCP::draw(void)
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
