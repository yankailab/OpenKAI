#ifndef AI_IO_FILEIO_H_
#define AI_IO_FILEIO_H_

#include "../Base/common.h"

using namespace std;

namespace kai
{

class FileIO
{
private:
	string m_name;
	string m_buf;
	fstream m_file;

//	ifstream m_if;
//	ofstream m_of;

public:
	FileIO(void);
	~FileIO(void);

	bool open(string* pName);
	void close(void);

	string* readAll(void);
	string* readLine(void);

	bool write(char* pBuf, int nByte);
	bool writeCRLF(void);

//	bool in(string* pFileName);
//	bool out(string* pFileName);

};

}

#endif
