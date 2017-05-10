#include "JSON.h"

namespace kai
{

JSON::JSON(void)
{
}

JSON::~JSON(void)
{
}

bool JSON::parse(string json)
{
	string error;
	const char* jsonstr = json.c_str();
	picojson::parse(m_JSON, jsonstr, jsonstr + strlen(jsonstr), &error);

	IF_F(!m_JSON.is<object>());

	return true;
}

bool JSON::v(string* pName, int* pVal)
{
	IF_F(!pName);
	IF_F(!m_JSON.is<object>());

	value var = m_JSON.get(*pName);
	IF_F(!var.is<int>());

	*pVal = (int) var.get<double>();
	return true;
}

bool JSON::v(string* pName, bool* pVal)
{
	IF_F(!pName);
	IF_F(!m_JSON.is<object>());

	value var = m_JSON.get(*pName);
	IF_F(!var.is<int>());

	*pVal = (((int) var.get<double>()) == 1 ? true : false);
	return true;
}

bool JSON::v(string* pName, uint64_t* pVal)
{
	IF_F(!pName);
	IF_F(!m_JSON.is<object>());

	value var = m_JSON.get(*pName);
	IF_F(!var.is<double>());

	*pVal = (uint64_t) var.get<double>();
	return true;
}

bool JSON::v(string* pName, double* pVal)
{
	IF_F(!pName);
	IF_F(!m_JSON.is<object>());

	value var = m_JSON.get(*pName);
	IF_F(!var.is<double>());

	*pVal = var.get<double>();
	return true;
}

bool JSON::v(string* pName, string* pVal)
{
	IF_F(!pName);
	IF_F(!m_JSON.is<object>());

	value var = m_JSON.get(*pName);
	IF_F(!var.is<string>());

	*pVal = var.get<string>();
	return true;
}

bool JSON::array(string* pName, value::array* pVal)
{
	IF_F(!pName);
	IF_F(!m_JSON.is<object>());

	value var = m_JSON.get(*pName);
	IF_F(!var.is<value::array>());

	*pVal = var.get<value::array>();
	return true;
}

bool JSON::array(string* pName, string* pVal, int nArray)
{
	value::array arr;
	IF_F(!array(pName, &arr));

	value::array::iterator it;
	int i = 0;
	for (it = arr.begin(); it != arr.end(); it++)
	{
		IF_T(i >= nArray);
		IF_F(!it->is<string>());
		pVal[i] = it->get<string>();
		i++;
	}

	return true;
}

bool JSON::array(string* pName, int* pVal, int nArray)
{
	value::array arr;
	IF_F(!array(pName, &arr));

	value::array::iterator it;
	int i = 0;
	for (it = arr.begin(); it != arr.end(); it++)
	{
		IF_T(i >= nArray);
		IF_F(!it->is<double>());
		pVal[i] = (int)it->get<double>();
		i++;
	}

	return true;
}

bool JSON::array(string* pName, double* pVal, int nArray)
{
	value::array arr;
	IF_F(!array(pName, &arr));

	value::array::iterator it;
	int i = 0;
	for (it = arr.begin(); it != arr.end(); it++)
	{
		IF_T(i >= nArray);
		IF_F(!it->is<double>());
		pVal[i] = it->get<double>();
		i++;
	}

	return true;
}

int JSON::checkErrorNum(void)
{
	int numError;
	string errorDesc;
	string err = "error";
	if (!v(&err, &numError))
	{
		return -1;
	}

	//TODO record error description

	return numError;
}

}
