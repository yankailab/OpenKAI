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
		m_pV = NULL;
		m_pC = NULL;
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

	bool _SATbase::decodeCNF(const string &cnf)
	{
		// cnf file input
		vector<string> vLines = splitBy(cnf, '\n');

		// find header
		for (int i = 0; i < vLines.size(); i++)
		{
			IF_CONT(vLines[i].empty());
			vector<string> vL = splitBy(vLines[i], ' ');
			IF_CONT(vL[0] == "c");
			IF_CONT(vL[0] != "p");

			// p cnf <VARIABLE_HWs> <clauses>
			IF_F(vL[1] != "cnf");

			m_nV = atoi(vL[2].c_str());
			m_nC = atoi(vL[3].c_str());
			break;
		}
		IF_F((m_nV == 0) || (m_nC == 0));

		// allocate
		m_nV++;	// index 0 is not used
		m_pV = new VARIABLE[m_nV];
		NULL_F(m_pV);

		m_pC = new CLAUSE[m_nC];
		NULL_F(m_pC);

		// decode clauses
		int iC = 0;
		for (int i = 0; i < vLines.size(); i++)
		{
			IF_CONT(vLines[i].empty());
			vector<string> vL = splitBy(vLines[i], ' ');
			IF_CONT(vL[0] == "c");
			IF_CONT(vL[0] == "p");

			CLAUSE* pC = &m_pC[iC];
			pC->clear();

			int iL = 0;
			while (vL[iL] != "0")
			{
				pC->addLiteral(atoi(vL[iL++].c_str()));

				// m_pC[iC].m_pL[iL] = atoi(vL[iL].c_str());
				// iL++;
			}
			IF_CONT(iL <= 0);

//			m_pC[iC].m_nL = iL;
			iC++;

			if (iC >= m_nC)
				break;
		}
		m_nC = iC;

		return true;
	}

	bool _SATbase::verify(void)
	{
		int nS = 0;
		for (int i = 0; i < m_nC; i++)
		{
			CLAUSE *pC = &m_pC[i];
			int nT = 0;

			for (int j = 0; j < pC->m_nL; j++)
			{
				int L = pC->getLiteral(j);
				IF_F(L == 0);	// error

				VARIABLE *pV = &m_pV[abs(L)];
				int v = pV->v();
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

		if (nS >= m_nC)
			return true;

		return false;
	}

	void _SATbase::printSolution(void)
	{
		string s = "";
		for (int i = 1; i < m_nV; i++)
		{
			int v = m_pV[i].v();
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
