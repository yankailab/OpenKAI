#ifndef OpenKAI_src_IO__File_H_
#define OpenKAI_src_IO__File_H_

#include "../Base/common.h"
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
	bool open(void);
	void close(void);
	void reset(void);

	int  read(uint8_t* pBuf, int nB);
	bool write(uint8_t* pBuf, int nB);
	bool writeLine(uint8_t* pBuf, int nB);

	bool open(string* pName);
	string* readAll(void);

private:
	string m_name;
	string m_buf;
	fstream m_file;
	int m_iByte;

};

}
#endif
