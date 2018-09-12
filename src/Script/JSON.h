
#ifndef OPENKAI_SRC_SCRIPT_JSON_H_
#define OPENKAI_SRC_SCRIPT_JSON_H_

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

	bool parse(string json);
	int checkErrorNum(void);

	bool v(const string& name, int* pVal);
	bool v(const string& name, bool* pVal);
	bool v(const string& name, uint8_t* pVal);
	bool v(const string& name, uint16_t* pVal);
	bool v(const string& name, uint32_t* pVal);
	bool v(const string& name, uint64_t* pVal);
	bool v(const string& name, double* pVal);
	bool v(const string& name, float* pVal);
	bool v(const string& name, string* pVal);

	int array(const string& name, int* pVal, int nArray);
	int array(const string& name, double* pVal, int nArray);
	int array(const string& name, string* pVal, int nArray);
	bool array(const string& name, value::array* pVal);

};

}
#endif
