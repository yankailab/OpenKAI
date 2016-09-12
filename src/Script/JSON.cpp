#include "JSON.h"

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

	if (!m_JSON.is<object>())
		return false;

	return true;
}

bool JSON::var(string name, int* val)
{
	if (!m_JSON.is<object>())
		return false;

	value var = m_JSON.get(name);
	if (!var.is<int>())
		return false;

	*val = (int) var.get<double>();
	return true;
}

bool JSON::var(string name, bool* val)
{
	if (!m_JSON.is<object>())
		return false;

	value var = m_JSON.get(name);
	if (!var.is<int>())
		return false;

	*val = (((int) var.get<double>()) == 1 ? true : false);
	return true;
}

bool JSON::var(string name, uint64_t* val)
{
	if (!m_JSON.is<object>())
		return false;

	value var = m_JSON.get(name);
	if (!var.is<double>())
		return false;

	*val = (uint64_t) var.get<double>();
	return true;
}

bool JSON::var(string name, double* val)
{
	if (!m_JSON.is<object>())
		return false;

	value var = m_JSON.get(name);
	if (!var.is<double>())
		return false;

	*val = var.get<double>();
	return true;
}

bool JSON::var(string name, string* val)
{
	if (!m_JSON.is<object>())
		return false;

	value var = m_JSON.get(name);
	if (!var.is<string>())
		return false;

	*val = var.get<string>();
	return true;
}

bool JSON::array(string name, value::array* val)
{
	if (!m_JSON.is<object>())
		return false;

	value var = m_JSON.get(name);
	if (!var.is<value::array>())
		return false;

	*val = var.get<value::array>();
	return true;

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

int JSON::checkErrorNum(void)
{
	int numError;
	string errorDesc;
	if (!var("error", &numError))
	{
		return -1;
	}

	//TODO record error description

	return numError;
}
