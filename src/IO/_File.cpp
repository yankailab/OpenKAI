#include "../Module/Kiss.h"
#include "_File.h"

namespace kai
{

	_File::_File(void)
	{
		m_ioType = io_file;
		m_name = "";
		m_buf = "";
		m_iByte = 0;
		m_ioStatus = io_closed;
	}

	_File::~_File(void)
	{
		close();
	}

	bool _File::init(const json& j)
	{
		IF_F(!this->_IObase::init(j));

		return true;
	}

	bool _File::open(const string& fName, ios_base::openmode mode)
	{
		IF__(m_ioStatus == io_opened, true);
		m_name = fName;

		return open(mode);
	}

	bool _File::open(ios_base::openmode mode)
	{
		IF_F(m_name.empty());
		IF__(m_ioStatus == io_opened, true);

		m_file.open(m_name.c_str(), mode);
		IF_F(!m_file.is_open());

		m_file.seekg(0, ios_base::beg);
		m_buf = "";
		m_ioStatus = io_opened;
		m_iByte = 0;

		return true;
	}

	void _File::close(void)
	{
		IF_(!m_file.is_open());
		IF_(m_ioStatus == io_closed);

		m_file.flush();
		m_file.close();
		m_ioStatus = io_closed;
	}

	int _File::read(uint8_t *pBuf, int nB)
	{
		if (!m_file.is_open())
			return -1;
		if (!pBuf)
			return -1;
		if (nB == 0)
			return 0;

		if (m_buf.size() == 0)
		{
			if (!readAll(NULL))
				return -1;
			m_iByte = 0;
		}

		if (m_iByte + nB >= m_buf.size())
		{
			nB = m_buf.size() - m_iByte;
		}
		if (nB <= 0)
			return 0;

		m_buf.copy((char *)pBuf, nB, m_iByte);
		m_iByte += nB;
		return nB;
	}

	bool _File::readAll(string *pStr)
	{
		IF_F(!m_file.is_open());

		m_file.seekg(0, ios_base::beg);
		m_buf.clear();
		string oneLine;

		while (m_file && !m_file.eof())
		{
			getline(m_file, oneLine);
			m_buf += oneLine;
		}

		if (pStr)
			*pStr = m_buf;

		return true;
	}

	bool _File::write(uint8_t *pBuf, int nB)
	{
		NULL_F(pBuf);
		IF_F(nB == 0);
		IF_F(!m_file.is_open());
		IF_F(!m_file.write((char *)pBuf, nB));

		return true;
	}

}
