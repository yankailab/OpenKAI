#include "File.h"
#include "../Script/Kiss.h"

namespace kai
{

File::File(void)
{
	IO();
	m_type = file;
	m_name = "";
	m_buf = "";
	m_iByte = 0;
}


File::~File(void)
{
}

bool File::init(void* pKiss)
{
	CHECK_F(!this->IO::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	string presetDir = "";
	F_INFO(pK->root()->o("APP")->v("presetDir", &presetDir));
	F_INFO(pK->v("fileName", &m_name));
	m_name = presetDir + m_name;
	m_status = closed;
	return true;
}

bool File::open(string* pName)
{
	NULL_F(pName);
	CHECK_T(m_status == opening);
	m_name = *pName;
	m_status = closed;

    return open();
}

bool File::open(void)
{
	CHECK_F(m_name.empty());
	CHECK_T(m_status == opening);

	m_file.open(m_name.c_str(),ios::in|ios::out|ios::app);
	CHECK_F(!m_file);
	m_file.seekg(0, ios_base::beg);
	m_buf = "";
	m_status = opening;
	m_iByte = 0;

	return true;
}

void File::close(void)
{
	CHECK_(!m_file.is_open());

	m_file.flush();
	m_file.close();
	m_status = closed;
}

int File::read(uint8_t* pBuf, int nByte)
{
	if(!m_file.is_open())return -1;
	if(!pBuf)return -1;
	if(nByte==0)return 0;

	if(m_buf.size()==0)
	{
		if(readAll()==NULL)return -1;
		m_iByte = 0;
	}

	if(m_iByte+nByte>=m_buf.size())nByte=m_buf.size()-m_iByte;
	if(nByte<=0)return 0;
	m_buf.copy((char*)pBuf, nByte, m_iByte);
	m_iByte += nByte;
	return nByte;
}

string* File::readAll(void)
{
	CHECK_N(!m_file.is_open());

	m_file.seekg(0, ios_base::beg);
	m_buf.clear();
	string oneLine;

	while(m_file && !m_file.eof())
    {
	    getline(m_file,oneLine);
		m_buf += oneLine;
    }

	return &m_buf;
}

bool File::write(uint8_t* pBuf, int nByte)
{
	NULL_F(pBuf);
	CHECK_F(nByte==0);
	CHECK_F(!m_file.is_open());

	CHECK_F(!m_file.write((char*)pBuf,nByte));
	return true;
}

bool File::writeLine(uint8_t* pBuf, int nByte)
{
	CHECK_F(!write(pBuf, nByte));

	const char crlf[] = "\x0d\x0a";
	CHECK_F(!m_file.write(crlf,2));

	return true;
}

}
