
#ifndef OpenKAI_src_Script_JSON_H_
#define OpenKAI_src_Script_JSON_H_

#include "../Base/common.h"
#include "picojson.h"

using namespace std;
using namespace picojson;

namespace kai
{

class JSON
{
private:
	picojson::value m_JSON;

public:
	JSON(void);
	~JSON(void);

	bool parse(string& json);
	int checkErrorNum(void);

	bool v(const string& name, picojson::value* pVal);

	bool v(const string& name, int* pVal);
	bool v(const string& name, bool* pVal);
	bool v(const string& name, uint8_t* pVal);
	bool v(const string& name, uint16_t* pVal);
	bool v(const string& name, uint32_t* pVal);
	bool v(const string& name, uint64_t* pVal);
	bool v(const string& name, double* pVal);
	bool v(const string& name, float* pVal);
	bool v(const string& name, string* pVal);

	bool v(const string& name, Scalar* pV);
	bool v(const string& name, vInt2* pV);
	bool v(const string& name, vInt3* pV);
	bool v(const string& name, vInt4* pV);
	bool v(const string& name, vFloat2* pV);
	bool v(const string& name, vFloat3* pV);
	bool v(const string& name, vFloat4* pV);
	bool v(const string& name, vDouble2* pV);
	bool v(const string& name, vDouble3* pV);
	bool v(const string& name, vDouble4* pV);

	bool array(const string& name, value::array* pVal);
	int a(const string& name, vector<int>* pVal);
	int a(const string& name, vector<double>* pVal);
	int a(const string& name, vector<float>* pVal);
	int a(const string& name, vector<string>* pVal);


};

}
#endif
