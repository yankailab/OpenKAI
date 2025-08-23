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
	}

	_SATbase::~_SATbase()
	{
	}

	int _SATbase::init(void *pKiss)
	{
		CHECK_(this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

///		pK->v("", &);
		string fName;
		pK->v("fName", &fName);

		string cnf;
		IF_F(!readFile(fName, &cnf, "\n"));
		IF_F(cnf.empty());

		return OK_OK;
	}

	int _SATbase::link(void)
	{
		CHECK_(this->_ModuleBase::link());
		Kiss *pK = (Kiss *)m_pKiss;

		return OK_OK;
	}

	int _SATbase::check(void)
	{
		return _ModuleBase::check();
	}

	bool _SATbase::decodeCNF(const string& cnf)
	{
		// cnf file input
		vector<string> vclauses = splitBy(cnf, '\n');

		for (int i = 0; i < vclauses.size(); i++)
		{
			IF_CONT(vclauses[i].empty());
			vector<string> vL = splitBy(vclauses[i], ' ');
			IF_CONT(vL[0] == "c");
			IF_CONT(vL[0] != "p");

			// p cnf <VARIABLE_HWs> <clauses>
			IF_F(vL[1] != "cnf");

			m_nV = atoi(vL[2].c_str());
			m_nC = atoi(vL[3].c_str());
			break;
		}
		IF_F((m_nV == 0) || (m_nC == 0));

		m_pC = new CLAUSE[m_nC];
		NULL_F(m_pC);

		int iC = 0;
		for (int i = 0; i < vclauses.size(); i++)
		{
			IF_CONT(vclauses[i].empty());
			vector<string> vL = splitBy(vclauses[i], ' ');
			IF_CONT(vL[0] == "c");
			IF_CONT(vL[0] == "p");

			int iL = 0;
			while (vL[iL] != "0")
			{
				m_pC[iC].m_pL[iL] = atoi(vL[iL].c_str());
				iL++;
			}
			IF_CONT(iL <= 0);

			m_pC[iC].m_nL = iL;
			iC++;

			if (iC >= m_nC)
				break;
		}
		m_nC = iC;

		return true;
	}


	void _SATbase::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);
	}

}
