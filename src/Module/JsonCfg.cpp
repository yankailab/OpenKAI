#include "JsonCfg.h"
#include "../Utility/utilFile.h"

namespace kai
{

	JsonCfg::JsonCfg(void)
	{
		m_jsonStr = "";
	}

	JsonCfg::~JsonCfg(void)
	{
	}

	bool JsonCfg::parseJsonFile(const string &fName)
	{
		IF_F(!readFromFile(fName));

		return parseJsonStr(m_jsonStr);
	}

	bool JsonCfg::readFromFile(const string &fName)
	{
		if (!readFile(fName, &m_jsonStr))
		{
			LOG_E("Cannot read file: " + fName);
			return false;
		}

		return true;
	}

	bool JsonCfg::saveToFile(const string &fName)
	{
		// TODO

		return true;
	}

	string JsonCfg::getJsonStr(void)
	{
		return m_jsonStr;
	}

	bool JsonCfg::parseJsonStr(const string &s)
	{
		m_jsonStr = s;
		delComment(&m_jsonStr);

		try
		{
			m_json = json::parse(m_jsonStr);
		}
		catch (const json::parse_error &e)
		{
			std::cerr << "Parse error: " << e.what() << "\n";
			return false;
		}

		return true;
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

	json& JsonCfg::getJson(void)
	{
		return m_json;
	}

	string JsonCfg::getName(void)
	{
		return "Jsonfg";
	}

}
