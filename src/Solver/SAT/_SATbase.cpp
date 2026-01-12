/*
 * _SATbase.cpp
 *
 *  Created on: Jul 11, 2025
 *      Author: yankai
 */

#include "_SATbase.h"

namespace kai
{

	_SATbase::_SATbase()
	{
		m_fName = "";
		clear();
	}

	_SATbase::~_SATbase()
	{
		clear();
	}

	int _SATbase::init(const json& j)
	{
		CHECK_(this->_ModuleBase::init(j));

		= j.value("fName", &m_fName);

		return true;
	}

	int _SATbase::link(const json& j, ModuleMgr* pM)
	{
		CHECK_(this->_ModuleBase::link(j, pM));

		return true;
	}

	int _SATbase::check(void)
	{
		return _ModuleBase::check();
	}

	void _SATbase::clear(void)
	{
		m_cnf = "";
		m_vV.clear();
		m_vC.clear();
	}

	bool _SATbase::readCNF(const string& fName, string* pCNF)
	{
		NULL_F(pCNF);

		IF_F(!readFile(fName, pCNF, "\n"));
		IF_F(pCNF->empty());

		return true;
	}

	bool _SATbase::decodeCNF(const string &cnf)
	{
		IF_F(cnf.empty());

		// cnf file input
		vector<string> vLines = splitBy(cnf, '\n');

		int i;

		// find header
		int nV;
		int nC;
		for (i = 0; i < vLines.size(); i++)
		{
			IF_CONT(vLines[i].empty());

			// p cnf <variables> <clauses>
			vector<string> vL = splitBy(vLines[i], ' ');
			IF_CONT(vL[0] == "c");
			IF_CONT(vL[0] != "p");

			IF_F(vL[1] != "cnf");

			nV = atoi(vL[2].c_str());
			nC = atoi(vL[3].c_str());
			break;
		}

		IF_F((nV <= 0) || (nC <= 0));

		// allocate
		VARIABLE v0; // index 0 is not used
		m_vV.push_back(v0);

		// clauses
		int iC = 0;
		for (i++; i < vLines.size(); i++)
		{
			IF_CONT(vLines[i].empty());

			vector<string> vL = splitBy(vLines[i], ' ');
			IF_CONT(vL[0] == "c");
			IF_CONT(vL[0] == "p");

			CLAUSE c;
			c.clear();

			int iL = 0;
			while (vL[iL] != "0")
			{
				c.addLiteral(atoi(vL[iL++].c_str()));
			}
			IF_CONT(iL <= 0);

			m_vC.push_back(c);
		}

		return true;
	}

	bool _SATbase::bSatisfied(void)
	{
		int nS = 0;
		for (int i = 0; i < m_vC.size(); i++)
		{
			CLAUSE *pC = &m_vC[i];
			int nT = 0;

			for (int j = 0; j < pC->m_vL.size(); j++)
			{
				int L = pC->getLiteral(j);
				IF_F(L == 0);	// error

				int v = m_vV[abs(L)].v();
				nT += (L < 0) ? 1 - v : v;

				if (nT > 0)
					break;
			}

			if (nT <= 0)
			{
				LOG_I("Unsatified: " + i2str(i));
			}
			else
			{
				nS++;
			}
		}

		if (nS >= m_vC.size())
			return true;

		return false;
	}

	void _SATbase::printSolution(void)
	{
		string s = "";
		for (int i = 1; i < m_vV.size(); i++)
		{
			int v = m_vV[i].v();
			s += i2str(v) + " ";
		}

		LOG_I("Assign: " + s);
	}

	void _SATbase::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);
	}

}
