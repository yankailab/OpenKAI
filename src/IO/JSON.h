#pragma once
#include "../Base/platform.h"
#include "picojson.h"

using namespace std;
using namespace picojson;

class JSON
{
private:
	picojson::value m_JSON;

public:
	JSON(void);
	~JSON(void);

	bool init(void);

	bool parse(string json);
	int checkErrorNum(void);

	bool getVal(string name,int* val);
	bool getVal(string name, bool* val);
	bool getVal(string name,uint64_t* val);
	bool getVal(string name,double* val);
	bool getVal(string name,string* val);

	bool getArray(string name,int* val);
	bool getArray(string name,value::array* val);




};

