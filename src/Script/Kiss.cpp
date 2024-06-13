#include "Kiss.h"

namespace kai
{

	Kiss::Kiss(void)
	{
		m_class = "";
		m_name = "";
		m_bNULL = false;
		m_pNULL = NULL;
		m_pParent = NULL;
		m_pInst = NULL;
	}

	Kiss::~Kiss(void)
	{
		for (Kiss *k : m_vChild)
		{
			IF_CONT(!k);
			delete k;
		}
		m_vChild.clear();

		if (!m_pParent)
		{
			IF_(m_bNULL);
			DEL(m_pNULL);
		}
	}

	bool Kiss::parse(const string& s)
	{
		IF_F(m_bNULL);

		// Create NULL instance
		if (m_pParent)
		{
			m_pNULL = this->m_pParent->m_pNULL;
		}
		else if (!m_pNULL)
		{
			m_pNULL = new Kiss();
			m_pNULL->m_bNULL = true;
			m_pNULL->m_pNULL = m_pNULL;
		}

		int k;
		std::string::size_type from, to;

		string sp = s;
		trim(&sp);
		delComment(&sp);

		do
		{
			// find the object start
			from = sp.find('{');
			if (from == std::string::npos)
				break;

			// find the paired bracket
			to = from + 1;
			k = 0;
			while (to < sp.length())
			{
				if (sp[to] == '{')
				{
					k++;
				}
				else if (sp[to] == '}')
				{
					if (k == 0)
						break;
					k--;
				}
				to++;
			}

			// the pair bracket not found
			IF_Fl(to == sp.length(), "Bracket not closed: " + sp);
//				return false;

			// check if it is a null object
			if (to > from + 1)
			{
				// create new obj
				string subStr = sp.substr(from + 1, to - from - 1);
				addChild(subStr);
			}

			sp.erase(from, to - from + 1);

		} while (1);

		string js = "{" + sp + "}";
		IF_Fl(!m_json.parse(js), "Parse failed:" + js);
//			return false;

		m_json.v("name", &m_name);
		m_json.v("class", &m_class);
		bool bON = true;
		m_json.v("bON", &bON);

//		IF_Fl(m_name.empty(), "Module name empty: " + js);
//		IF_Fl(m_class.empty(), "Module class empty: " + js);
		IF_Fl(find(m_name), "Module already existed" + js);

		return bON;
	}

	bool Kiss::addChild(const string& s)
	{
		IF_F(m_bNULL);

		Kiss *pChild = new Kiss();
		pChild->m_pParent = this;
		if (!pChild->parse(s))
		{
			delete pChild;
			return false;
		}

		m_vChild.push_back(pChild);
		return true;
	}

	Kiss *Kiss::child(const string &name)
	{
		IF__(m_bNULL, m_pNULL);

		for (Kiss *k : m_vChild)
		{
			if (k->m_name == name)
				return k;
		}

		return m_pNULL;
	}

	Kiss *Kiss::child(int i)
	{
		IF__(i < 0, m_pNULL);
		IF__(i >= m_vChild.size(), m_pNULL);

		return m_vChild[i];
	}

	Kiss *Kiss::root(void)
	{
		Kiss *pRoot = this;

		while (pRoot->m_pParent)
			pRoot = pRoot->m_pParent;

		return pRoot;
	}

	Kiss *Kiss::parent(void)
	{
		IF__(!m_pParent, m_pNULL);

		return m_pParent;
	}

	Kiss *Kiss::find(const string &name)
	{
		vector<string> vName = splitBy(name, '.');
		IF_N(vName.empty());

		Kiss *pK = root();
		for (string n : vName)
			pK = pK->child(n);

		IF_N(pK->empty());
		return pK;
	}

	void *Kiss::getInst(const string &name)
	{
		Kiss *pK = find(name);
		NULL_N(pK);
		return pK->m_pInst;
	}

	bool Kiss::empty(void)
	{
		return m_bNULL;
	}

	JSON *Kiss::json(void)
	{
		return &m_json;
	}

	void Kiss::trim(string *pS)
	{
		std::string::size_type k;

		// do NOT delete white spaces as gst pipeline use it for parameter separations

		k = pS->find('\r');
		while (k != std::string::npos)
		{
			pS->erase(k, 1);
			k = pS->find('\r');
		}

		k = pS->find('\t');
		while (k != std::string::npos)
		{
			pS->erase(k, 1);
			k = pS->find('\t');
		}
	}

	void Kiss::delComment(string *pStr)
	{
		std::string::size_type cFrom;
		std::string::size_type cTo;
		string commentFrom = "/*";
		string commentTo = "*/";

		cFrom = pStr->find(commentFrom);
		while (cFrom != std::string::npos)
		{
			cTo = pStr->find(commentTo, cFrom + commentFrom.length());
			if (cTo == std::string::npos)
			{
				cTo = pStr->length() - commentTo.length();
			}

			pStr->erase(cFrom, cTo - cFrom + commentTo.length());
			cFrom = pStr->find(commentFrom);
		}
	}

}
