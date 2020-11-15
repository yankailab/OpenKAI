#ifndef OpenKAI_src_IO__File_H_
#define OpenKAI_src_IO__File_H_

#include "_IOBase.h"

using namespace std;

namespace kai
{

class _File : public _IOBase
{
public:
	_File(void);
	~_File(void);

	bool init(void* pKiss);
	bool open(ios_base::openmode mode = ios::in | ios::out | ios::app);
	void close(void);
	void reset(void);

	int  read(uint8_t* pBuf, int nB);
	bool write(uint8_t* pBuf, int nB);
	bool writeLine(uint8_t* pBuf, int nB);

	bool open(string* pName, ios_base::openmode mode = ios::in | ios::out | ios::app);
	string* readAll(void);

public:
	string m_name;
	string m_buf;
	fstream m_file;
	int m_iByte;

};

}
#endif
