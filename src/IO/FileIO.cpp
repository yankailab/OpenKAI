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

bool FileIO::open(string* pName)
{
	NULL_F(pName);
	m_name = *pName;

	m_file.open(pName->c_str(),ios::in|ios::out);
	CHECK_F(!m_file);
	m_buf = "";

    return true;
}

void FileIO::close(void)
{
	CHECK_(!m_file.is_open());

	m_file.flush();
	m_file.close();
}

string* FileIO::readAll(void)
{
	CHECK_N(!m_file.is_open());

	string oneLine;
	m_file.seekg(0, ios_base::beg);
	m_buf.clear();

	while(!m_file.eof())
    {
	    getline(m_file,oneLine);
		m_buf += oneLine;
    }

	return &m_buf;
}

string* FileIO::readLine(void)
{
	CHECK_N(!m_file.is_open());

	m_buf.clear();
	CHECK_N(!getline(m_file,m_buf));

	return &m_buf;
}

bool FileIO::write(char* pBuf, int nByte)
{
	NULL_F(pBuf);
	CHECK_F(nByte==0);
	CHECK_F(!m_file.is_open());

	CHECK_F(!m_file.write(pBuf,nByte));
	return true;
}

bool FileIO::writeCRLF(void)
{
	CHECK_F(!m_file.is_open());

	const char crlf[] = "\x0d\x0a";
	CHECK_F(!m_file.write(crlf,2));

	return true;
}


//bool FileIO::in(string* pFileName)
//{
//	NULL_F(pFileName);
//	m_name = *pFileName;
//
//	m_if.open(pFileName->c_str(),ios::in);
//    CHECK_F(!m_if);
//
//	return true;
//}
//
//
//bool FileIO::out(string* pFileName)
//{
//	NULL_F(pFileName);
//	m_name = *pFileName;
//
//	m_of.open(pFileName->c_str(),ios::out);
//    CHECK_F(!m_of);
//
//	return true;
//}




}
