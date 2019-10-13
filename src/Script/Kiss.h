#ifndef OpenKAI_src_Script_Kiss_H_
#define OpenKAI_src_Script_Kiss_H_

#include "../Base/common.h"
#include "../Base/BASE.h"
#include "JSON.h"

using namespace std;

#define N_CHILDREN 128

namespace kai
{

class Kiss
{
public:
	Kiss(void);
	~Kiss(void);

	bool parse(string* pStr);
	Kiss* o(const string& name);
	JSON* json(void);
	Kiss** getClassItr(const string& className);
	Kiss** getChildItr(void);
	void* getChildInst(const string& name);
	Kiss* root(void);
	Kiss* parent(void);
	bool empty(void);

	template <typename T> bool v(const string& name, T* pVal)
	{
		return m_json.v(name, pVal);
	}

	template <typename T> int a(const string& name, vector<T>* pVal)
	{
		return m_json.a(name, pVal);
	}

private:
	void trim(string* pStr);
	void delComment(string* pStr);
	bool addChild(string* pStr);

public:
	string m_class;
	string m_name;
	JSON m_json;
	BASE* m_pInst;

	Kiss* m_pParent;
	int m_nChild;
	Kiss* m_pChild[N_CHILDREN];

	Kiss* m_pNULL;
	bool m_bNULL;

	Kiss* m_ppItr[N_CHILDREN];

};

}
#endif
