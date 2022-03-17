#ifndef OpenKAI_src_Utility_utilFile_H_
#define OpenKAI_src_Utility_utilFile_H_

#include "../Base/platform.h"
#include "../Base/macro.h"
#include "../Base/constant.h"
#include "../Script/Kiss.h"

namespace kai
{

inline bool readFile(const string& fName, string* pStr, string delim = "")
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

inline bool parseKiss(const string& fName, Kiss* pK)
{
	NULL_F(pK);

	string s;
	IF_F(!readFile(fName, &s));
	IF_F(s.empty());
	
	return pK->parse(&s);
}

}
#endif