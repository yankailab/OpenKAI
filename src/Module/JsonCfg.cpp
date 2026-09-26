#include "JsonCfg.h"
#include "../Utility/utilFile.h"

namespace kai
{

	JsonCfg::JsonCfg(void)
	{
	}

	JsonCfg::~JsonCfg(void)
	{
	}

	bool JsonCfg::readFromFile(const string &fName)
	{
		string jStr;
		IF_F(!readFile(fName, &jStr));
		IF_F(!parseStr(jStr));

		m_name = fName;
		return true;
	}

	bool JsonCfg::parseStr(const string &s)
	{
		string jStr = s;
		delComment(&jStr);

		m_json.clear();

		try
		{
			m_json = json::parse(jStr);
		}
		catch (const json::parse_error &e)
		{
			std::cerr << "Parse error: " << e.what() << "\n";
			return false;
		}

		return true;
	}

	void JsonCfg::setJson(const json &j)
	{
		m_json = j;
	}

	bool JsonCfg::saveToFile(const string &fName)
	{
		string n = fName;
		if (n.empty())
		{
			n = m_name;
		}
		if (n.empty())
		{
			return false;
		}

		string jStr = m_json.dump(m_nDumpSpace);
		IF_F(!writeFile(n, jStr));

		return true;
	}

	json *JsonCfg::getJson(void)
	{
		return &m_json;
	}

	string JsonCfg::getStr(void)
	{
		return m_json.dump(m_nDumpSpace);
	}

	string JsonCfg::getFileName(void) const
	{
		return m_name;
	}

	void JsonCfg::setNdumpSpace(int nD)
	{
		IF_(nD <= 0);
		m_nDumpSpace = nD;
	}

	void JsonCfg::delComment(string *pStr)
	{
		NULL_(pStr);

		std::string::size_type cFrom;
		std::string::size_type cTo;
		string commentFrom = "/*";
		string commentTo = "*/";

		cFrom = pStr->find(commentFrom);
		while (cFrom != std::string::npos)
		{
			cTo = pStr->find(commentTo, cFrom + commentFrom.length());
			if (cTo == std::string::npos)
			{
				cTo = pStr->length() - commentTo.length();
			}

			pStr->erase(cFrom, cTo - cFrom + commentTo.length());
			cFrom = pStr->find(commentFrom);
		}
	}

}
