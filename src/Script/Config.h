
#ifndef OPENKAI_SRC_IO_CONFIG_H_
#define OPENKAI_SRC_IO_CONFIG_H

#include "../Base/platform.h"
#include "JSON.h"
#include <string>

using namespace std;

#define NUM_CHILDREN 128

class Config
{
public:
	Config(void);
	~Config(void);

	bool parse(string str);
	Config* o(string name);
	JSON* json(void);
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
	bool addChild(string str);

public:
	string	m_name;
	JSON	m_json;
	int		m_nChild;
	Config*	m_pChild[NUM_CHILDREN];

	Config* m_pNULL;
	bool	m_bNULL;
};

#endif
