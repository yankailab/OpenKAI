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
	m_status = unknown;
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
	m_name = *pName;
	m_status = closed;

    return open();
}

bool File::open(void)
{
	CHECK_F(m_name.empty());

	m_file.open(m_name.c_str(),ios::in|ios::out|ios::app);
	CHECK_F(!m_file);
	m_buf = "";

	m_status = opening;
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

    m_file.read((char*)pBuf,nByte);

	return m_file.gcount();
}

string* File::readAll(void)
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

string* File::readLine(void)
{
	CHECK_N(!m_file.is_open());

	m_buf.clear();
	CHECK_N(!getline(m_file,m_buf));

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
