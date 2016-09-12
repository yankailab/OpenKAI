
#ifndef OPENKAI_SRC_IO_JSON_H_
#define OPENKAI_SRC_IO_JSON_H

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

	bool parse(string json);
	int checkErrorNum(void);

	bool var(string name,int* val);
	bool var(string name, bool* val);
	bool var(string name,uint64_t* val);
	bool var(string name,double* val);
	bool var(string name,string* val);

//	bool array(string name,int* val);
	bool array(string name,value::array* val);


};

#endif
