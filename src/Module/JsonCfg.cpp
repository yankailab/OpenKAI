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

	bool JsonCfg::readFromFile(const string &fName)
	{
		IF_F(!readFile(fName, &m_jsonStr));

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

	bool JsonCfg::parseJson(const string &s)
	{
		m_jsonStr = s;
		delComment(&m_jsonStr);

		m_json = json::parse(m_jsonStr, /*callback*/ nullptr, /*allow_exceptions*/ false);
		IF_F(m_json.is_discarded());

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

	json JsonCfg::getJson(void)
	{
		return m_json;
	}

	json JsonCfg::getJson(const string &s)
	{
		auto jP = json::json_pointer(s);
		IF__(!m_json.contains(jP), nullptr);

		json j = m_json.at(jP);
		IF__(!j.is_object(), nullptr);

		return j;
	}

}
