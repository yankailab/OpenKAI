#include "FileIO.h"

FileIO::FileIO(void)
{
}


FileIO::~FileIO(void)
{
}

bool FileIO::open(string fileName)
{
	m_buf = "";
	string oneLine;

	m_file.open(fileName.c_str(),ios::in);
    if(!m_file)
	{
		return false;
    }

	while(!m_file.eof())
    {
	    getline(m_file,oneLine);
		m_buf += oneLine;
    }
	m_file.close();

	return true;
}

string FileIO::getContent(void)
{
	return m_buf;
}
