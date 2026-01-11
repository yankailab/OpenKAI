#ifndef OpenKAI_src_Module_JsonCfg_H_
#define OpenKAI_src_Module_JsonCfg_H_

#include "../Base/common.h"
#include "../Dependencies/json.hpp"

using namespace std;
using namespace nlohmann;

namespace kai
{

	class JsonCfg
	{
	public:
		JsonCfg(void);
		~JsonCfg(void);

		bool readFromFile(const string &fName);
		bool saveToFile(const string &fName);

		string getJsonStr(void);
		bool parseJson(const string& s);

		json getJson(void);
		json getJson(const string& s);

	private:
		void delComment(string *pStr);

	private:
		string m_jsonStr;
		json m_json;

	};

}
#endif
