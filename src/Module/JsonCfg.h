#ifndef OpenKAI_src_Module_JsonCfg_H_
#define OpenKAI_src_Module_JsonCfg_H_

#include "../Base/common.h"
#include "../Dependencies/json.h"

using namespace std;
using namespace nlohmann;

template <typename T>
bool jKv(const json &j, const std::string &key, T &v, bool bLog = false)
{
	if (!j.is_object())
	{
		if (bLog)
		{
			LOG(INFO) << "JSON is not an object: " + key;
		}
		return false;
	}

	auto it = j.find(key);
	if (it == j.end())
	{
		if (bLog)
		{
			LOG(INFO) << "Cannot find: " + key;
		}
		return false;
	}

	try
	{
		v = it->get<T>();
		return true;
	}
	catch (const json::type_error &e)
	{
		if (bLog)
		{
			LOG(INFO) << "Type error: " + key + ": " + std::string(e.what());
		}
		return false;
	}
	catch (const json::out_of_range &e)
	{
		if (bLog)
		{
			LOG(INFO) << "Out-of-range: " + key + ": " + std::string(e.what());
		}
		return false;
	}
	catch (const json::exception &e)
	{
		if (bLog)
		{
			LOG(INFO) << "JSON exception: " + key + ": " + std::string(e.what());
		}
		return false;
	}

	return true;
}

template <typename T1, typename T2>
bool jKv(const json &j, const std::string &key, T2 &v, bool bLog = false)
{
	std::vector<T1> vT;
	if (jKv(j, key, vT, bLog))
	{
		v = vT;
		return true;
	}

	return false;
}

inline const json& jK(const json &j, const std::string &key, bool bLog = false)
{
	static const json jNull = nullptr;

	if (!j.is_object())
	{
		if (bLog)
		{
			LOG(INFO) << "JSON is not an object: " + key;
		}
		return jNull;
	}

	auto it = j.find(key);
	if (it == j.end())
	{
		if (bLog)
		{
			LOG(INFO) << "Cannot find: " + key;
		}
		return jNull;
	}

	return j.at(key);
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

		json& getJson(void);
		string getName(void);

	protected:
		void delComment(string *pStr);

	protected:
		string m_jsonStr;
		json m_json;
	};

}
#endif
