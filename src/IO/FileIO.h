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
	ifstream m_if;
	ofstream m_of;

public:
	FileIO(void);
	~FileIO(void);

	string* readAll(string* pFileName);

	bool in(string* pFileName);
	string* readLine(void);

	bool out(string* pFileName);
	bool write(char* pBuf, int nByte);
	bool CRLF(void);

	void close(void);
};

}

#endif
