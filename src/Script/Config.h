
#ifndef OPENKAI_SRC_IO_CONFIG_H_
#define OPENKAI_SRC_IO_CONFIG_H

#include "../Base/platform.h"
#include "JSON.h"
#include <string>

using namespace std;

#define NUM_CHILDREN 1024

class Config
{
public:
	Config(void);
	~Config(void);

	bool parse(string str);
	Config* obj(string name);
	JSON* json(void);
	bool empty(void);

	bool var(string name,int* val);
	bool var(string name, bool* val);
	bool var(string name,uint64_t* val);
	bool var(string name,double* val);
	bool var(string name,string* val);

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
