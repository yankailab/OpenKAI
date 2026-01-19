#ifndef OpenKAI_src_Module_JsonCfg_H_
#define OpenKAI_src_Module_JsonCfg_H_

#include "../Base/common.h"
#include "../Dependencies/json.h"

using namespace std;
using namespace nlohmann;

template <typename T>
bool jKv(const json &j, const std::string &key, T &v)
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

	try
	{
		v = it->get<T>();
		return true;
	}
	catch (const json::type_error &e)
	{
		LOG(INFO) << "Type error: " + key + ": " + std::string(e.what());
		return false;
	}
	catch (const json::out_of_range &e)
	{
		LOG(INFO) << "Out-of-range: " + key + ": " + std::string(e.what());
		return false;
	}
	catch (const json::exception &e)
	{
		LOG(INFO) << "JSON exception: " + key + ": " + std::string(e.what());
		return false;
	}

	return true;
}

template <typename T1, typename T2>
bool jKv(const json &j, const std::string &key, T2 &v)
{
	std::vector<T1> vT;
	if (jKv(j, key, vT))
	{
		v = vT;
		return true;
	}

	return false;
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
