
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

	bool v(string* pName, int* pVal);
	bool v(string* pName, bool* pVal);
	bool v(string* pName, uint64_t* pVal);
	bool v(string* pName, double* pVal);
	bool v(string* pName, string* pVal);

	int array(string* pName, int* pVal, int nArray);
	int array(string* pName, double* pVal, int nArray);
	int array(string* pName, string* pVal, int nArray);
	bool array(string* pName, value::array* pVal);

};

}
#endif
