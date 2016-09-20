
#ifndef OPENKAI_SRC_IO_CONFIG_H_
#define OPENKAI_SRC_IO_CONFIG_H

#include "../Base/platform.h"
#include "JSON.h"
#include <string>

using namespace std;

#define NUM_CHILDREN 128

namespace kai
{

class Config
{
public:
	Config(void);
	~Config(void);

	bool parse(string* pStr);
	Config* o(string name);
	JSON* json(void);
	Config** getClassItr(string* pClassName);
	Config** getChildItr(void);
	Config* getChildByName(string* pName);
	void* getChildInstByName(string* pName);
	Config* root(void);
	Config* parent(void);
	bool empty(void);

	bool v(string name,int* val);
	bool v(string name, bool* val);
	bool v(string name,uint64_t* val);
	bool v(string name,double* val);
	bool v(string name,string* val);

//	bool array(string name,int* val);
	bool array(string name,value::array* val);


private:
	void trim(string* pStr);
	bool addChild(string* pStr);

public:
	string	m_class;
	string	m_name;
	JSON	m_json;
	void*	m_pInst;

	Config* m_pParent;
	int		m_nChild;
	Config*	m_pChild[NUM_CHILDREN];

	Config* m_pNULL;
	bool	m_bNULL;

	Config* m_ppItr[NUM_CHILDREN];

};

}

#endif
