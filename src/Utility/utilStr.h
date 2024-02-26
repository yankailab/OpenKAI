#ifndef OpenKAI_src_Utility_utilStr_H_
#define OpenKAI_src_Utility_utilStr_H_

#include "../Base/platform.h"
#include "../Base/macro.h"
#include "../Base/constant.h"

inline string deleteNonASCII(const char *pStr)
{
	string asc = "";
	if (!pStr)
		return asc;

	int i = 0;
	while (pStr[i] != 0)
	{
		char c = pStr[i++];
		IF_CONT(c < 0);
		IF_CONT(c >= 128);
		asc += c;
	}
	return asc;
}

inline string deleteNonNumber(const char *pStr)
{
	string asc = "";
	if (!pStr)
		return asc;

	int i = 0;
	while (pStr[i] != 0)
	{
		char c = pStr[i++];
		IF_CONT(c < 48);
		IF_CONT(c > 57);
		asc += c;
	}
	return asc;
}

inline string replace(string str, const string &strF, const string &strR)
{
	return str.replace(str.find(strF), strF.length(), strR);
}

inline string getFileDir(string file)
{
	return file.erase(file.find_last_of('/') + 1, string::npos);
}

inline string checkDirName(string &dir)
{
	if (dir.at(dir.length() - 1) != '/')
		dir.push_back('/');

	return dir;
}

inline string getFileExt(string &file)
{
	size_t extPos = file.find_last_of(".");
	if (extPos == std::string::npos)
		return "";
	return file.substr(extPos);
}

#define UTIL_BUF 64

inline string i2str(int v)
{
	char buf[UTIL_BUF];
	snprintf(buf, UTIL_BUF, "%d", v);
	return string(buf);
}

inline string li2str(long v)
{
	char buf[UTIL_BUF];
	snprintf(buf, UTIL_BUF, "%ld", v);
	return string(buf);
}

inline string f2str(float v)
{
	char buf[UTIL_BUF];
	snprintf(buf, UTIL_BUF, "%.3f", v);
	return string(buf);
}

inline string f2str(float v, int nDigit)
{
	char buf[UTIL_BUF];
	string format = "%." + i2str(nDigit) + "f";
	snprintf(buf, UTIL_BUF, format.c_str(), (float)v);
	return string(buf);
}

inline string lf2str(double v)
{
	char buf[UTIL_BUF];
	snprintf(buf, UTIL_BUF, "%.3f", (double)v);
	return string(buf);
}

inline string lf2str(double v, int nDigit)
{
	char buf[UTIL_BUF];
	string format = "%." + i2str(nDigit) + "f";
	snprintf(buf, UTIL_BUF, format.c_str(), (double)v);
	return string(buf);
}

inline void trimJson(string *pStr)
{
	unsigned int idx = pStr->rfind('}', pStr->size()) + 1;
	if (pStr->size() > idx)
	{
		pStr->erase(idx);
	}

	idx = pStr->find('{', 0);
	if (idx != string::npos && idx > 0)
	{
		pStr->erase(0, idx - 1);
	}
}

inline vector<string> splitBy(string str, char c)
{
	vector<string> v;
	string buf = "";
	stringstream ss;

	ss << str;
	while (getline(ss, buf, c))
	{
		v.push_back(buf);
	}

	return v;
}

#endif
