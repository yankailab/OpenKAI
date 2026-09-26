#ifndef OpenKAI_src_Module_JsonCfg_H_
#define OpenKAI_src_Module_JsonCfg_H_

#include "../Base/common.h"
#include "../Dependencies/json.h"
#include <optional>

using namespace std;
using namespace nlohmann;

template <typename T>
bool jKv(const json &j, const string &key, T &v, bool bLog = false)
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
bool jKv(const json &j, const string &key, T2 &v, bool bLog = false)
{
	vector<T1> vT;
	if (jKv(j, key, vT, bLog))
	{
		v = vT;
		return true;
	}

	return false;
}

// Copy only supplied coefficients, preserving defaults for short config arrays.
template <typename T1, typename Scalar, int N, int Options, int MaxRows, int MaxCols>
bool jKv(const json &j, const string &key,
		 Eigen::Matrix<Scalar, N, 1, Options, MaxRows, MaxCols> &v, bool bLog = false)
{
	vector<T1> values;
	IF_F(!jKv(j, key, values, bLog));

	const auto n = std::min(values.size(), static_cast<size_t>(v.size()));
	for (size_t i = 0; i < n; ++i)
		v[static_cast<Eigen::Index>(i)] = static_cast<Scalar>(values[i]);

	return true;
}

template <typename T>
bool jKv(const json &j, const string &key, std::optional<T> &value)
{
	IF_F(!j.is_object() || !j.contains(key));
	if (j.at(key).is_null())
	{
		value.reset();
		return true;
	}

	T configured{};
	if (!::jKv(j, key, configured))
		throw std::invalid_argument("Invalid Orbbec option: " + key);
	value = configured;
	return true;
}

inline json *jK(json &j, const std::string &key, bool bLog = false)
{
	if (!j.is_object())
	{
		if (bLog)
		{
			LOG(INFO) << "JSON is not an object: " + key;
		}
		return nullptr;
	}

	auto it = j.find(key);
	if (it == j.end())
	{
		if (bLog)
		{
			LOG(INFO) << "Cannot find: " + key;
		}
		return nullptr;
	}

	return &(*it);
}

inline const json *jK(const json &j, const std::string &key, bool bLog = false)
{
	if (!j.is_object())
	{
		if (bLog)
		{
			LOG(INFO) << "JSON is not an object: " + key;
		}
		return nullptr;
	}

	auto it = j.find(key);
	if (it == j.end())
	{
		if (bLog)
		{
			LOG(INFO) << "Cannot find: " + key;
		}
		return nullptr;
	}

	return &(*it);
}

namespace kai
{

	class JsonCfg
	{
	public:
		JsonCfg(void);
		~JsonCfg(void);

		// input
		bool readFromFile(const string &fName);
		bool parseStr(const string &s);
		void setJson(const json &j);

		// output
		bool saveToFile(const string &fName = "");
		json *getJson(void);
		string getStr(void);
		string getFileName(void) const;
		void setNdumpSpace(int nD);

	protected:
		void delComment(string *pStr);

	protected:
		string m_name = ""; // .json file name

		json m_json;
		int m_nDumpSpace = 4;
	};

}
#endif
