#include "Config.h"

namespace kai
{

Config::Config(void)
{
	m_nChild = 0;
	m_class = "";
	m_name = "";
	m_bNULL = false;
	m_pNULL = NULL;
	m_pParent = NULL;
	m_pInst = NULL;
}

Config::~Config(void)
{
}

bool Config::parse(string* pStr)
{
	if (m_bNULL)return false;

	//Create NULL instance
	if(m_pParent)
	{
		m_pNULL = this->m_pParent->m_pNULL;
	}
	else if (!m_pNULL)
	{
		m_pNULL = new Config();
		m_pNULL->m_bNULL = true;
	}

	int k;
	std::string::size_type from, to;

	trim(pStr);

	do
	{
		//find the object start
		from = pStr->find('{');
		if (from == std::string::npos)
			break;

		//find the paired bracket
		to = from + 1;
		k = 0;
		while (to < pStr->length())
		{
			if ((*pStr)[to] == '{')
			{
				k++;
			}
			else if ((*pStr)[to] == '}')
			{
				if (k == 0)
					break;
				k--;
			}
			to++;
		}

		//the pair bracket not found
		if (to == pStr->length())
			return false;

		//check if it is a null object
		if (to > from + 1)
		{
			//create new obj
			string subStr = pStr->substr(from + 1, to - from - 1);
			if (!addChild(&subStr))
				return false;
		}

		pStr->erase(from, to - from + 1);

	} while (1);

	if (!m_json.parse("{" + (*pStr) + "}"))
		return false;

	string name;
	name = "name";
	m_json.v(&name, &m_name);

	name = "class";
	m_json.v(&name, &m_class);

	return true;
}

void Config::trim(string* pStr)
{
	std::string::size_type k;

	k = pStr->find(' ');
	while (k != std::string::npos)
	{
		pStr->erase(k,1);
		k = pStr->find(' ');
	}

	k = pStr->find('\r');
	while (k != std::string::npos)
	{
		pStr->erase(k,1);
		k = pStr->find('\r');
	}

	k = pStr->find('\t');
	while (k != std::string::npos)
	{
		pStr->erase(k,1);
		k = pStr->find('\t');
	}

}

bool Config::addChild(string* pStr)
{
	if (m_bNULL)
		return false;
	if (m_nChild >= NUM_CHILDREN)
		return false;

	m_pChild[m_nChild] = new Config();
	Config* pChild = m_pChild[m_nChild];
	m_nChild++;

	pChild->m_pParent = this;
	return pChild->parse(pStr);
}

Config* Config::o(string name)
{
	if (name.empty())
		return NULL;
	if (m_bNULL)
		return m_pNULL;

	for (int i = 0; i < m_nChild; i++)
	{
		if (m_pChild[i]->m_name == name)
			return m_pChild[i];
	}

	return m_pNULL;
}

Config* Config::root(void)
{
	Config* pRoot = this;

	while(pRoot->m_pParent)
	{
		pRoot = pRoot->m_pParent;
	}

	return pRoot;
}

Config* Config::parent(void)
{
	return m_pParent;
}

JSON* Config::json(void)
{
	return &m_json;
}

Config** Config::getClassItr(string* pClassName)
{
	if(pClassName==NULL)return NULL;
	if(pClassName->empty())return NULL;

	int i;
	int nFound = 0;

	//Find list with the class name
	for(i=0; i<m_nChild; i++)
	{
		Config* pC = m_pChild[i];
		if(pC->m_class != (*pClassName))continue;

		m_ppItr[nFound]=pC;
		nFound++;

		if(nFound == NUM_CHILDREN-1)break;
	}

	m_ppItr[nFound]=NULL;
	nFound++;

	return m_ppItr;
}

Config** Config::getChildItr(void)
{
	m_pChild[m_nChild]=NULL;

	return m_pChild;
}

Config* Config::getChildByName(string* pName)
{
	if(pName==NULL)return NULL;
	if(pName->empty())return NULL;

	for(int i=0; i<m_nChild; i++)
	{
		Config* pC = m_pChild[i];
		if(pC->m_name == (*pName))
		{
			return pC;
		}
	}

	return NULL;
}

void* Config::getChildInstByName(string* pName)
{
	void* pC = getChildByName(pName);
	if(!pC)return NULL;

	return ((Config*)pC)->m_pInst;
}

bool Config::empty(void)
{
	return m_bNULL;
}

bool Config::v(string name, int* val)
{
	return m_json.v(&name, val);
}

bool Config::v(string name, bool* val)
{
	return m_json.v(&name, val);
}

bool Config::v(string name, uint64_t* val)
{
	return m_json.v(&name, val);
}

bool Config::v(string name, double* val)
{
	return m_json.v(&name, val);
}

bool Config::v(string name, string* val)
{
	return m_json.v(&name, val);
}

bool Config::array(string name, value::array* val)
{
	return m_json.array(&name, val);
}

}
