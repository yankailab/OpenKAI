#pragma once

#include "platform.h"

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


