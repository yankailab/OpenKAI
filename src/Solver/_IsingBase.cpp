/*
 * _IsingBase.cpp
 *
 *  Created on: Feb 26, 2026
 *      Author: yankai
 */

#include "_IsingBase.h"

namespace kai
{

	_IsingBase::_IsingBase()
	{
		m_fName = "";
		clear();
	}

	_IsingBase::~_IsingBase()
	{
		clear();
	}

	bool _IsingBase::init(const json &j)
	{
		IF_F(!this->_ModuleBase::init(j));

		jKv(j, "fName", m_fName);

		return true;
	}

	bool _IsingBase::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_ModuleBase::link(j, pM));

		return true;
	}

	bool _IsingBase::check(void)
	{
		return _ModuleBase::check();
	}

	void _IsingBase::clear(void)
	{
		m_cnf = "";
		m_spinAssign.clearAll();
		m_vJw.clear();
	}

	bool _IsingBase::readCNF(const string &fName, string *pCNF)
	{
		NULL_F(pCNF);

		IF_F(!readFile(fName, pCNF, "\n"));
		IF_F(pCNF->empty());

		return true;
	}

	bool _IsingBase::decodeCNF(const string &cnf)
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

			// p cnf <ISING_VARs> <ISING_TERMs>
			vector<string> vL = splitBy(vLines[i], ' ');
			IF_CONT(vL[0] == "c");
			IF_CONT(vL[0] != "p");

			IF_F(vL[1] != "cnf");

			nV = atoi(vL[2].c_str());
			nC = atoi(vL[3].c_str());
			break;
		}

		IF_F((nV <= 0) || (nC <= 0));

		// Ising terms
		int iC = 0;
		for (i++; i < vLines.size(); i++)
		{
			IF_CONT(vLines[i].empty());

			vector<string> vL = splitBy(vLines[i], ' ');
			IF_CONT(vL[0] == "c");
			IF_CONT(vL[0] == "p");

			ISING_JW c;
			c.clear();

			// int iL = 0;
			// while (vL[iL] != "0")
			// {
			// 	c.addLiteral(atoi(vL[iL++].c_str()));
			// }
			// IF_CONT(iL <= 0);

			m_vJw.push_back(c);
		}

		return true;
	}

	double _IsingBase::energy(void)
	{
		double e = 0;
		for (int i = 0; i < m_vJw.size(); i++)
		{
			ISING_JW *pJw = &m_vJw[i];
			vBit *pWb = &pJw->m_w;

			int jwS = 1;
			size_t nB = pWb->nBits();
			size_t iBp = pWb->findFirstOne();
			while (iBp < nB)
			{
				int8_t s = m_spinAssign.get(iBp);
				if (s < 0)
					break;

				jwS *= (s << 1) - 1; // 0/1 to -1/1

				iBp = pWb->findNextOne(iBp);
			}

			e += pJw->m_J * jwS;
		}

		LOG_I("Energy: " + lf2str(e));
		return e;
	}

	void _IsingBase::printSolution(void)
	{
		string s = "";
		// for (int i = 1; i < m_vV.size(); i++)
		// {
		// 	int v = m_vV[i].v();
		// 	s += i2str(v) + " ";
		// }

		LOG_I("Assign: " + s);
	}

	void _IsingBase::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);
	}

}
