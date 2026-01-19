#ifndef OpenKAI_src_Module_JsonCfg_H_
#define OpenKAI_src_Module_JsonCfg_H_

#include "../Base/common.h"
#include "../Dependencies/json.h"

using namespace std;
using namespace nlohmann;

#include <string>
#include <type_traits>

template <class T>
inline constexpr bool is_string_v =
	std::is_same_v<std::remove_cv_t<std::remove_reference_t<T>>, std::string>;

template <typename T>
bool jVar(const json &j, const std::string &key, T &v)
{
	if (!j.is_object())
	{
		LOG(INFO) << "JSON is not an object: " + key;
		return false;
	}

	auto it = j.find(key);
	if (it == j.end())
	{
		LOG(INFO) << "Cannot find: " + key;
		return false;
	}

	// const T *p = it->get_ptr<const T *>();
	// if (p == nullptr)
	// {
	// 	LOG(INFO) << "Incorrect type: " + key;
	// 	return false;
	// }

	// v = *p;
	return true;
}

template <typename T1, typename T2>
bool jVec(const json &j, const std::string &key, T2 &v)
{
	if (!j.is_object())
	{
		LOG(INFO) << "JSON is not an object: " + key;
		return false;
	}

	auto it = j.find(key);
	if (it == j.end())
	{
		LOG(INFO) << "Cannot find: " + key;
		return false;
	}

	if (!it->is_array())
	{
		LOG(INFO) << "Not an array: " + key;
		return false;
	}

	std::vector<T1> vT;
	vT.reserve(it->size());

	if constexpr (is_string_v<T1>)
	{
		for (const auto &e : *it)
		{
			if (!e.is_string())
			{
				LOG(INFO) << "Array element is not a string";
				vT.clear();
				break;
			}

			vT.push_back(e.get<T1>());
		}
	}
	else
	{
		for (const auto &e : *it)
		{
			if (!e.is_number())
			{
				LOG(INFO) << "Array element is not a number";
				vT.clear();
				break;
			}

			vT.push_back(e.get<T1>());
		}
	}

	v = vT;

	return true;
}

namespace kai
{

	class JsonCfg
	{
	public:
		JsonCfg(void);
		~JsonCfg(void);

		bool parseJsonFile(const string &fName);
		bool readFromFile(const string &fName);
		bool saveToFile(const string &fName);

		string getJsonStr(void);
		bool parseJsonStr(const string &s);

		json getJson(void);
		json getJson(const string &s);

		string getName(void);

	private:
		void delComment(string *pStr);

	protected:
		string m_jsonStr;
		json m_json;
	};

}
#endif
