#include "JSON.h"

namespace kai
{

	JSON::JSON(void)
	{
	}

	JSON::~JSON(void)
	{
	}

	bool JSON::parse(const string &json)
	{
		// string error;
		// const char *jsonstr = json.c_str();
		// picojson::parse(m_JSON, jsonstr, jsonstr + strlen(jsonstr), &error);

		// IF_F(!m_JSON.is<object>());

		return true;
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

		// TODO record error description

		return numError;
	}

	// void JSON::setJSON(const picojson::value& json)
	// {
	// 	m_JSON = json;
	// }

	// bool JSON::v(const string &name, picojson::value *pVal)
	// {
	// 	IF_F(!m_JSON.is<object>());

	// 	value var = m_JSON.get(name);
	// 	IF_F(!var.is<object>());

	// 	*pVal = var;
	// 	return true;
	// }

	bool JSON::v(const string &name, int *pVal)
	{
		// IF_F(!m_JSON.is<object>());

		// value var = m_JSON.get(name);
		// IF_F(!var.is<double>());

		// *pVal = (int)var.get<double>();
		return true;
	}

	bool JSON::v(const string &name, bool *pVal)
	{
		// IF_F(!m_JSON.is<object>());

		// value var = m_JSON.get(name);
		// IF_F(!var.is<double>());

		// *pVal = (((int)var.get<double>()) != 0 ? true : false);
		return true;
	}

	bool JSON::v(const string &name, uint8_t *pVal)
	{
		// IF_F(!m_JSON.is<object>());

		// value var = m_JSON.get(name);
		// IF_F(!var.is<double>());

		// *pVal = (uint8_t)var.get<double>();
		return true;
	}

	bool JSON::v(const string &name, uint16_t *pVal)
	{
		// IF_F(!m_JSON.is<object>());

		// value var = m_JSON.get(name);
		// IF_F(!var.is<double>());

		// *pVal = (uint16_t)var.get<double>();
		return true;
	}

	bool JSON::v(const string &name, uint32_t *pVal)
	{
		// IF_F(!m_JSON.is<object>());

		// value var = m_JSON.get(name);
		// IF_F(!var.is<double>());

		// *pVal = (uint32_t)var.get<double>();
		return true;
	}

	bool JSON::v(const string &name, uint64_t *pVal)
	{
		// IF_F(!m_JSON.is<object>());

		// value var = m_JSON.get(name);
		// IF_F(!var.is<double>());

		// *pVal = (uint64_t)var.get<double>();
		return true;
	}

	bool JSON::v(const string &name, double *pVal)
	{
		// IF_F(!m_JSON.is<object>());

		// value var = m_JSON.get(name);
		// IF_F(!var.is<double>());

		// *pVal = var.get<double>();
		return true;
	}

	bool JSON::v(const string &name, float *pVal)
	{
		// IF_F(!m_JSON.is<object>());

		// value var = m_JSON.get(name);
		// IF_F(!var.is<double>());

		// *pVal = (float)var.get<double>();
		return true;
	}

	bool JSON::v(const string &name, string *pVal)
	{
		// IF_F(!m_JSON.is<object>());

		// value var = m_JSON.get(name);
		// IF_F(!var.is<string>());

		// *pVal = var.get<string>();
		return true;
	}

	bool JSON::v(const string &name, vFloat2 *pV)
	{
		// value::array arr;
		// IF_F(!array(name, &arr));

		// value::array::iterator it;

		// it = arr.begin();
		// IF_F(it == arr.end());
		// IF_F(!it->is<double>());
		// pV->x = (float)it->get<double>();

		// it++;
		// IF_F(it == arr.end());
		// IF_F(!it->is<double>());
		// pV->y = (float)it->get<double>();

		return true;
	}

	bool JSON::v(const string &name, vFloat3 *pV)
	{
		// value::array arr;
		// IF_F(!array(name, &arr));

		// value::array::iterator it;

		// it = arr.begin();
		// IF_F(it == arr.end());
		// IF_F(!it->is<double>());
		// pV->x = (float)it->get<double>();

		// it++;
		// IF_F(it == arr.end());
		// IF_F(!it->is<double>());
		// pV->y = (float)it->get<double>();

		// it++;
		// IF_F(it == arr.end());
		// IF_F(!it->is<double>());
		// pV->z = (float)it->get<double>();

		return true;
	}

	bool JSON::v(const string &name, vFloat4 *pV)
	{
		// value::array arr;
		// IF_F(!array(name, &arr));

		// value::array::iterator it;

		// it = arr.begin();
		// IF_F(it == arr.end());
		// IF_F(!it->is<double>());
		// pV->x = (float)it->get<double>();

		// it++;
		// IF_F(it == arr.end());
		// IF_F(!it->is<double>());
		// pV->y = (float)it->get<double>();

		// it++;
		// IF_F(it == arr.end());
		// IF_F(!it->is<double>());
		// pV->z = (float)it->get<double>();

		// it++;
		// IF_F(it == arr.end());
		// IF_F(!it->is<double>());
		// pV->w = (float)it->get<double>();

		return true;
	}

#ifdef USE_OPENCV
	bool JSON::v(const string &name, Scalar *pV)
	{
		// value::array arr;
		// IF_F(!array(name, &arr));

		// value::array::iterator it;

		// it = arr.begin();
		// IF_F(it == arr.end());
		// IF_F(!it->is<double>());
		// pV->val[0] = it->get<double>();

		// it++;
		// IF_F(it == arr.end());
		// IF_F(!it->is<double>());
		// pV->val[1] = it->get<double>();

		// it++;
		// IF_F(it == arr.end());
		// IF_F(!it->is<double>());
		// pV->val[2] = it->get<double>();

		// it++;
		// IF_F(it == arr.end());
		// IF_F(!it->is<double>());
		// pV->val[3] = it->get<double>();

		return true;
	}
#endif

	bool JSON::v(const string &name, vInt2 *pV)
	{
		// value::array arr;
		// IF_F(!array(name, &arr));

		// value::array::iterator it;

		// it = arr.begin();
		// IF_F(it == arr.end());
		// IF_F(!it->is<double>());
		// pV->x = (int)it->get<double>();

		// it++;
		// IF_F(it == arr.end());
		// IF_F(!it->is<double>());
		// pV->y = (int)it->get<double>();

		return true;
	}

	bool JSON::v(const string &name, vInt3 *pV)
	{
		// value::array arr;
		// IF_F(!array(name, &arr));

		// value::array::iterator it;

		// it = arr.begin();
		// IF_F(it == arr.end());
		// IF_F(!it->is<double>());
		// pV->x = (int)it->get<double>();

		// it++;
		// IF_F(it == arr.end());
		// IF_F(!it->is<double>());
		// pV->y = (int)it->get<double>();

		// it++;
		// IF_F(it == arr.end());
		// IF_F(!it->is<double>());
		// pV->z = (int)it->get<double>();

		return true;
	}

	bool JSON::v(const string &name, vInt4 *pV)
	{
		// value::array arr;
		// IF_F(!array(name, &arr));

		// value::array::iterator it;

		// it = arr.begin();
		// IF_F(it == arr.end());
		// IF_F(!it->is<double>());
		// pV->x = (int)it->get<double>();

		// it++;
		// IF_F(it == arr.end());
		// IF_F(!it->is<double>());
		// pV->y = (int)it->get<double>();

		// it++;
		// IF_F(it == arr.end());
		// IF_F(!it->is<double>());
		// pV->z = (int)it->get<double>();

		// it++;
		// IF_F(it == arr.end());
		// IF_F(!it->is<double>());
		// pV->w = (int)it->get<double>();

		return true;
	}

	bool JSON::v(const string &name, vDouble2 *pV)
	{
		// value::array arr;
		// IF_F(!array(name, &arr));

		// value::array::iterator it;

		// it = arr.begin();
		// IF_F(it == arr.end());
		// IF_F(!it->is<double>());
		// pV->x = it->get<double>();

		// it++;
		// IF_F(it == arr.end());
		// IF_F(!it->is<double>());
		// pV->y = it->get<double>();

		return true;
	}

	bool JSON::v(const string &name, vDouble3 *pV)
	{
		// value::array arr;
		// IF_F(!array(name, &arr));

		// value::array::iterator it;

		// it = arr.begin();
		// IF_F(it == arr.end());
		// IF_F(!it->is<double>());
		// pV->x = it->get<double>();

		// it++;
		// IF_F(it == arr.end());
		// IF_F(!it->is<double>());
		// pV->y = it->get<double>();

		// it++;
		// IF_F(it == arr.end());
		// IF_F(!it->is<double>());
		// pV->z = it->get<double>();

		return true;
	}

	bool JSON::v(const string &name, vDouble4 *pV)
	{
		// value::array arr;
		// IF_F(!array(name, &arr));

		// value::array::iterator it;

		// it = arr.begin();
		// IF_F(it == arr.end());
		// IF_F(!it->is<double>());
		// pV->x = it->get<double>();

		// it++;
		// IF_F(it == arr.end());
		// IF_F(!it->is<double>());
		// pV->y = it->get<double>();

		// it++;
		// IF_F(it == arr.end());
		// IF_F(!it->is<double>());
		// pV->z = it->get<double>();

		// it++;
		// IF_F(it == arr.end());
		// IF_F(!it->is<double>());
		// pV->w = it->get<double>();

		return true;
	}

	// bool JSON::array(const string &name, value::array *pVal)
	// {
	// 	IF_F(!m_JSON.is<object>());

	// 	value var = m_JSON.get(name);
	// 	IF_F(!var.is<value::array>());

	// 	*pVal = var.get<value::array>();
	// 	return true;
	// }

	int JSON::a(const string &name, vector<string> *pVal)
	{
		// value::array arr;
		// IF__(!array(name, &arr), -1);

		// value::array::iterator it;
		// int i = 0;
		// for (it = arr.begin(); it != arr.end(); it++)
		// {
		// 	IF_CONT(!it->is<string>());
		// 	pVal->push_back(it->get<string>());
		// 	i++;
		// }

//		return i;
		return 0;
	}

	int JSON::a(const string &name, vector<int> *pVal)
	{
		// value::array arr;
		// IF__(!array(name, &arr), -1);

		// value::array::iterator it;
		// int i = 0;
		// for (it = arr.begin(); it != arr.end(); it++)
		// {
		// 	IF_CONT(!it->is<double>());
		// 	pVal->push_back((int)it->get<double>());
		// 	i++;
		// }

//		return i;
		return 0;
	}

	int JSON::a(const string &name, vector<double> *pVal)
	{
		// value::array arr;
		// IF__(!array(name, &arr), -1);

		// value::array::iterator it;
		// int i = 0;
		// for (it = arr.begin(); it != arr.end(); it++)
		// {
		// 	IF_CONT(!it->is<double>());
		// 	pVal->push_back(it->get<double>());
		// 	i++;
		// }

//		return i;
		return 0;
	}

	int JSON::a(const string &name, vector<float> *pVal)
	{
		// value::array arr;
		// IF__(!array(name, &arr), -1);

		// value::array::iterator it;
		// int i = 0;
		// for (it = arr.begin(); it != arr.end(); it++)
		// {
		// 	IF_CONT(!it->is<double>());
		// 	pVal->push_back((float)it->get<double>());
		// 	i++;
		// }

//		return i;
		return 0;
	}

	int JSON::a(const string &name, vector<vInt4> *pVal)
	{
		// value::array arr;
		// IF__(!array(name, &arr), -1);

		// value::array::iterator it;
		// int i = 0;
		// for (it = arr.begin(); it != arr.end(); it++)
		// {
		// 	IF_CONT(!it->is<value::array>());

		// 	vInt4 v;
		// 	v.clear();

		// 	value::array aV = it->get<value::array>();
		// 	value::array::iterator vItr;

		// 	vItr = aV.begin();
		// 	if (vItr != aV.end() && vItr->is<double>())
		// 		v.x = (int)vItr->get<double>();

		// 	vItr++;
		// 	if (vItr != aV.end() && vItr->is<double>())
		// 		v.y = (int)vItr->get<double>();

		// 	vItr++;
		// 	if (vItr != aV.end() && vItr->is<double>())
		// 		v.z = (int)vItr->get<double>();

		// 	vItr++;
		// 	if (vItr != aV.end() && vItr->is<double>())
		// 		v.w = (int)vItr->get<double>();

		// 	pVal->push_back(v);
		// 	i++;
		// }

		// return i;


		return 0;
	}

}
