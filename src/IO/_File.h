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

		bool init(void *pKiss);
		void close(void);
		void reset(void);

		bool open(ios_base::openmode mode = ios::in | ios::out | ios::app);
		bool open(const string &fName, ios_base::openmode mode = ios::in | ios::out | ios::app);
		int read(uint8_t *pBuf, int nB);
		bool readAll(string *pStr);
		bool write(uint8_t *pBuf, int nB);
		bool writeLine(uint8_t *pBuf, int nB);

	public:
		string m_name;
		string m_buf;
		fstream m_file;
		int m_iByte;
	};

}
#endif
