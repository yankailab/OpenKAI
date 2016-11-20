
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

	bool v(string* pName, int* val);
	bool v(string* pName, bool* val);
	bool v(string* pName, uint64_t* val);
	bool v(string* pName, double* val);
	bool v(string* pName, string* val);

//	bool array(string name,int* val);
	bool array(string* pName,value::array* val);


};

}
#endif
