#ifndef OpenKAI_src_Utility_utilFile_H_
#define OpenKAI_src_Utility_utilFile_H_

#include "../Base/platform.h"
#include "../Base/macro.h"
#include "../Base/constant.h"

namespace kai
{
	inline bool readFile(const string &fName, string *pStr, string delim = "")
	{
		NULL_F(pStr);

		fstream f;
		f.open(fName.c_str(), ios::in);
		IF_F(!f.is_open());
		f.seekg(0, ios_base::beg);

		*pStr = "";
		while (f && !f.eof())
		{
			string s;
			getline(f, s);
			*pStr += s + delim;
		}

		f.close();

		return true;
	}

	inline bool writeFile(const string &fName, const string &str)
	{
		fstream f;
		f.open(fName.c_str(), ios::out);
		IF_F(!f.is_open());

		f.seekg(0, ios_base::beg);

		IF_F(!f.write(str.c_str(), str.length()));

		f.flush();
		f.close();

		return true;
	}
}
#endif