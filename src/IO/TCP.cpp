#include "TCP.h"
#include "../Script/Kiss.h"

namespace kai
{

TCP::TCP(void)
{
	IO();
	m_type = tcp;
	m_status = unknown;
	m_pPeer = NULL;
}


TCP::~TCP(void)
{
}

bool TCP::init(void* pKiss)
{
	CHECK_F(!this->IO::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	//peer
	m_pPeer = new _peer();
	F_ERROR_F(m_pPeer->init(pK));

	m_status = closed;
	return true;
}

bool TCP::open(void)
{
	NULL_F(m_pPeer);
	m_pPeer->start();

	m_status = opening;
    return true;
}

void TCP::close(void)
{

	m_status = closed;
}

int TCP::read(uint8_t* pBuf, int nByte)
{
	if(!m_pPeer)return -1;
	if(!m_pPeer->m_bConnected)return -1;
	if(!pBuf)return -1;
	if(nByte==0)return 0;

//    m_file.read((char*)pBuf,nByte);

	return nByte;
}

bool TCP::write(uint8_t* pBuf, int nByte)
{
	NULL_F(pBuf);
	CHECK_F(nByte==0);
	NULL_F(m_pPeer);
	CHECK_F(m_pPeer->m_bConnected);

//	CHECK_F(!m_file.write((char*)pBuf,nByte));
	return true;
}

bool TCP::writeLine(uint8_t* pBuf, int nByte)
{
	CHECK_F(!write(pBuf, nByte));

	const char crlf[] = "\x0d\x0a";
//	CHECK_F(!m_file.write(crlf,2));

	return true;
}

}
