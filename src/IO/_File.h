#ifndef OpenKAI_src_IO__File_H_
#define OpenKAI_src_IO__File_H_

#include "_IObase.h"

using namespace std;

namespace kai
{

	class _File : public _IObase
	{
	public:
		_File(void);
		~_File(void);

		virtual bool init(const json& j);

		bool open(ios_base::openmode mode = ios::in | ios::out | ios::app);
		bool open(const string &fName, ios_base::openmode mode = ios::in | ios::out | ios::app);
		void close(void);

		int read(uint8_t *pBuf, int nB);
		bool write(uint8_t *pBuf, int nB);

	protected:
		bool readAll(string *pStr);

	private:
		string m_name;
		string m_buf;
		fstream m_file;
		int m_iByte;
	};

}
#endif
