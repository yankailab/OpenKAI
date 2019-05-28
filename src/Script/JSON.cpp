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

bool JSON::v(const string& name, int* pVal)
{
	IF_F(!m_JSON.is<object>());

	value var = m_JSON.get(name);
	IF_F(!var.is<int>());

	*pVal = (int) var.get<double>();
	return true;
}

bool JSON::v(const string& name, bool* pVal)
{
	IF_F(!m_JSON.is<object>());

	value var = m_JSON.get(name);
	IF_F(!var.is<int>());

	*pVal = (((int) var.get<double>()) != 0 ? true : false);
	return true;
}

bool JSON::v(const string& name, uint8_t* pVal)
{
	IF_F(!m_JSON.is<object>());

	value var = m_JSON.get(name);
	IF_F(!var.is<int>());

	*pVal = (uint8_t) var.get<double>();
	return true;
}

bool JSON::v(const string& name, uint16_t* pVal)
{
	IF_F(!m_JSON.is<object>());

	value var = m_JSON.get(name);
	IF_F(!var.is<int>());

	*pVal = (uint16_t) var.get<double>();
	return true;
}

bool JSON::v(const string& name, uint32_t* pVal)
{
	IF_F(!m_JSON.is<object>());

	value var = m_JSON.get(name);
	IF_F(!var.is<int>());

	*pVal = (uint32_t) var.get<double>();
	return true;
}

bool JSON::v(const string& name, uint64_t* pVal)
{
	IF_F(!m_JSON.is<object>());

	value var = m_JSON.get(name);
	IF_F(!var.is<int>());

	*pVal = (uint64_t) var.get<double>();
	return true;
}

bool JSON::v(const string& name, double* pVal)
{
	IF_F(!m_JSON.is<object>());

	value var = m_JSON.get(name);
	IF_F(!var.is<double>());

	*pVal = var.get<double>();
	return true;
}

bool JSON::v(const string& name, float* pVal)
{
	IF_F(!m_JSON.is<object>());

	value var = m_JSON.get(name);
	IF_F(!var.is<double>());

	*pVal = (float)var.get<double>();
	return true;
}

bool JSON::v(const string& name, string* pVal)
{
	IF_F(!m_JSON.is<object>());

	value var = m_JSON.get(name);
	IF_F(!var.is<string>());

	*pVal = var.get<string>();
	return true;
}

bool JSON::array(const string& name, value::array* pVal)
{
	IF_F(!m_JSON.is<object>());

	value var = m_JSON.get(name);
	IF_F(!var.is<value::array>());

	*pVal = var.get<value::array>();
	return true;
}

int JSON::array(const string& name, string* pVal, int nArray)
{
	value::array arr;
	IF__(!array(name, &arr),-1);

	value::array::iterator it;
	int i = 0;
	for (it = arr.begin(); it != arr.end(); it++)
	{
		if(i >= nArray)return i;
		IF_CONT(!it->is<string>());
		pVal[i] = it->get<string>();
		i++;
	}

	return i;
}

int JSON::array(const string& name, int* pVal, int nArray)
{
	value::array arr;
	IF__(!array(name, &arr),-1);

	value::array::iterator it;
	int i = 0;
	for (it = arr.begin(); it != arr.end(); it++)
	{
		if(i >= nArray)return i;
		IF_CONT(!it->is<int>());
		pVal[i] = (int)it->get<double>();
		i++;
	}

	return i;
}

int JSON::array(const string& name, double* pVal, int nArray)
{
	value::array arr;
	IF__(!array(name, &arr),-1);

	value::array::iterator it;
	int i = 0;
	for (it = arr.begin(); it != arr.end(); it++)
	{
		if(i >= nArray)return i;
		IF_CONT(!it->is<double>());
		pVal[i] = it->get<double>();
		i++;
	}

	return i;
}

int JSON::array(const string& name, float* pVal, int nArray)
{
	value::array arr;
	IF__(!array(name, &arr),-1);

	value::array::iterator it;
	int i = 0;
	for (it = arr.begin(); it != arr.end(); it++)
	{
		if(i >= nArray)return i;
		IF_CONT(!it->is<double>());
		pVal[i] = (float)it->get<double>();
		i++;
	}

	return i;
}

int JSON::checkErrorNum(void)
{
	int numError;
	string errorDesc;
	string err = "error";
	if (!v(err, &numError))
	{
		return -1;
	}

	//TODO record error description

	return numError;
}

}
