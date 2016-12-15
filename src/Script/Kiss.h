#ifndef OPENKAI_SRC_SCRIPT_KISS_H_
#define OPENKAI_SRC_SCRIPT_KISS_H_

#include "../Base/common.h"
#include "../Base/BASE.h"
#include "JSON.h"

using namespace std;

#define NUM_CHILDREN 128

namespace kai
{

class Kiss
{
public:
	Kiss(void);
	~Kiss(void);

	bool parse(string* pStr);
	Kiss* o(string name);
	JSON* json(void);
	Kiss** getClassItr(string* pClassName);
	Kiss** getChildItr(void);
	Kiss* getChildByName(string* pName);
	void* getChildInstByName(string* pName);
	Kiss* root(void);
	Kiss* parent(void);
	bool empty(void);

	bool v(string name, int* val);
	bool v(string name, bool* val);
	bool v(string name, uint64_t* val);
	bool v(string name, double* val);
	bool v(string name, string* val);

	bool array(string name, value::array* val);

private:
	void trim(string* pStr);
	bool addChild(string* pStr);

public:
	string m_class;
	string m_name;
	JSON m_json;
	BASE* m_pInst;

	Kiss* m_pParent;
	int m_nChild;
	Kiss* m_pChild[NUM_CHILDREN];

	Kiss* m_pNULL;
	bool m_bNULL;

	Kiss* m_ppItr[NUM_CHILDREN];

};

}

#endif
