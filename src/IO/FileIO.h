#ifndef AI_IO_FILEIO_H_
#define AI_IO_FILEIO_H_

#include "../Base/common.h"

using namespace std;

class FileIO
{
private:
	string m_buf;
	ifstream m_file;

public:
	FileIO(void);
	~FileIO(void);

	bool open(string fileName);
	string getContent(void);

};

#endif
