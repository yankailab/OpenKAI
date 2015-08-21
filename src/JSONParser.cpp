#include "JSONParser.h"


JSONParser::JSONParser(void)
{
}


JSONParser::~JSONParser(void)
{
}

bool JSONParser::parse(string json)
{
	string error;
	const char* jsonstr = json.c_str();
	picojson::parse(m_JSON, jsonstr, jsonstr+strlen(jsonstr), &error);

	if(!m_JSON.is<object>())
	{
//		m_logger.addLog(0,LOG_ERROR,"parse().parse():"+error);
		return false;
	}
	return true;
}

bool JSONParser::getVal(string name, int* val)
{
	if(m_JSON.is<object>())
	{
		value var = m_JSON.get(name);
		if(var.is<int>())
		{
			*val = (int)var.get<double>();
			return true;
		}
    }

	return false;
}
/*
bool JSONParser::getVal(string name, int32_t* val)
{
	if(m_JSON.is<object>())
	{
		value var = m_JSON.get(name);
		if(var.is<double>())
		{
			*val = (int32_t)var.get<double>();
			return true;
		}
    }

	return false;
}
*/
bool JSONParser::getVal(string name, double* val)
{
	if(m_JSON.is<object>())
	{
		value var = m_JSON.get(name);
		if(var.is<double>())
		{
			*val = var.get<double>();
			return true;
		}
    }

	return false;
}

bool JSONParser::getVal(string name, string* val)
{
	if(m_JSON.is<object>())
	{
		value var = m_JSON.get(name);
		if(var.is<string>())
		{
			*val = var.get<string>();
			return true;
		}
    }

	return false;
}

bool JSONParser::getArray(string name, array* val)
{
	if(m_JSON.is<object>())
	{
		value var = m_JSON.get(name);
		if(var.is<array>())
		{
			*val = var.get<array>();
			return true;
		}
    }

	return false;

	//TODO
/*      array arr = m_JSON.get<array>();
      array::iterator it;
      for (it = arr.begin(); it != arr.end(); it++) {
        object obj = it->get<object>();
        cout << obj["user_login_id"].to_str() << ": " << obj["text"].to_str() << endl;
      }

	  return false;
*/
}

int JSONParser::checkErrorNum(void)
{
	int numError;
	string errorDesc;
	if(!getVal("error",&numError))
	{
//		m_logger.addLog(0,LOG_ERROR,"checkErrorNum():getVal()");
		return -1;
	}

	//TODO record error description

	return numError;
}
