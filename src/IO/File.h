#ifndef AI_IO_FILE_H_
#define AI_IO_FILE_H_

#include "../Base/common.h"
#include "IO.h"

using namespace std;

namespace kai
{

class File : public IO
{
public:
	File(void);
	~File(void);

	//common
	bool init(void* pKiss);
	bool open(void);
	void close(void);

	int  read(uint8_t* pBuf, int nByte);
	bool write(uint8_t* pBuf, int nByte);
	bool writeLine(uint8_t* pBuf, int nByte);

	//File
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
