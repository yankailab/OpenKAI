#include "TCP.h"
#include "../Script/Kiss.h"

namespace kai
{

TCP::TCP(void)
{
	IO();
	m_type = tcp;
	m_pPeer = NULL;
	m_pServer = NULL;
}


TCP::~TCP(void)
{
	if(m_pPeer)
	{
		m_pPeer->complete();
		delete m_pPeer;
	}
}

bool TCP::init(void* pKiss)
{
	CHECK_F(!this->IO::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	string iName = "";
	F_INFO(pK->v("_server", &iName));
	if(!iName.empty())
	{
		m_pServer = (_server*) (pK->root()->getChildInstByName(&iName));
		NULL_F(m_pServer);
		CHECK_F(!m_pServer->registerInstPeer(&pK->m_name,&m_pPeer));
	}
	else
	{
		m_pPeer = new _peer();
		F_ERROR_F(m_pPeer->init(pK));
	}

	m_status = closed;
	return true;
}

bool TCP::open(void)
{
	if(m_pPeer)
	{
		if(m_pPeer->m_bClient)
		{
			CHECK_F(m_pPeer->start());
		}
	}

	m_status = opening;
    return true;
}

void TCP::close(void)
{
	m_status = closed;
	NULL_(m_pPeer);
	m_pPeer->close();
}

int TCP::read(uint8_t* pBuf, int nByte)
{
	if(!m_pPeer)return -1;
    return m_pPeer->read(pBuf,nByte);
}

bool TCP::write(uint8_t* pBuf, int nByte)
{
	NULL_F(m_pPeer);
	return m_pPeer->write(pBuf,nByte);
}

bool TCP::writeLine(uint8_t* pBuf, int nByte)
{
	CHECK_F(!write(pBuf, nByte));

	const char crlf[] = "\x0d\x0a";
	return write((uint8_t*)crlf, 2);
}

}
