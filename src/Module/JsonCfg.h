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

		bool parseJsonFile(const string& fName);
		bool readFromFile(const string &fName);
		bool saveToFile(const string &fName);

		string getJsonStr(void);
		bool parseJsonStr(const string& s);

		json getJson(void);
		json getJson(const string& s);

		string getName(void);

	private:
		void delComment(string *pStr);

	protected:
		string m_jsonStr;
		json m_json;

	};

}
#endif
