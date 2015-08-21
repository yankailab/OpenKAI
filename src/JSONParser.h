#pragma once
#include "include/picojson.h"

using namespace std;
using namespace picojson;

class JSONParser
{
private:
	picojson::value m_JSON;

public:
	JSONParser(void);
	~JSONParser(void);

	bool init(void);

	bool parse(string json);
	int checkErrorNum(void);

//	bool getVal(string name,int* val);
	bool getVal(string name,int32_t* val);
	bool getVal(string name,double* val);
	bool getVal(string name,string* val);

	bool getArray(string name,int* val);
	bool getArray(string name,array* val);




};

