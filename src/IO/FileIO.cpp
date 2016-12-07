#include "FileIO.h"

namespace kai
{

FileIO::FileIO(void)
{
	m_name = "";
	m_buf = "";
}


FileIO::~FileIO(void)
{
}

string* FileIO::readAll(string* pFileName)
{
	NULL_F(pFileName);
	m_name = *pFileName;

	m_buf = "";
	string oneLine;

	m_if.open(pFileName->c_str(),ios::in);
    CHECK_F(!m_if);

	while(!m_if.eof())
    {
	    getline(m_if,oneLine);
		m_buf += oneLine;
    }
	m_if.close();

	return &m_buf;
}

bool FileIO::in(string* pFileName)
{
	NULL_F(pFileName);
	m_name = *pFileName;

	m_if.open(pFileName->c_str(),ios::in);
    CHECK_F(!m_if);

	return true;
}

string* FileIO::readLine(void)
{
	CHECK_N(!m_if);
	CHECK_N(m_if.eof());

	m_buf.clear();
	CHECK_N(!getline(m_if,m_buf));

	return &m_buf;
}

bool FileIO::out(string* pFileName)
{
	NULL_F(pFileName);
	m_name = *pFileName;

	m_of.open(pFileName->c_str(),ios::out);
    CHECK_F(!m_of);

	return true;
}

bool FileIO::write(char* pBuf, int nByte)
{
	NULL_F(pBuf);
	CHECK_F(nByte==0);
	CHECK_F(!m_of);

	m_of.write(pBuf,nByte);

	return true;
}

bool FileIO::CRLF(void)
{
	CHECK_F(!m_of);

	const char crlf[] = "\x0d\x0a";
	m_of.write(crlf,2);

	return true;
}


void FileIO::close(void)
{
	if(m_of)
	{
		m_of.flush();
		m_of.close();
	}

	if(m_if)
	{
		m_if.close();
	}

}

}
