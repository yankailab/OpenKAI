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
		m_nV = 0;
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

		size_t i;

		// find header
		int nT;
		for (i = 0; i < vLines.size(); i++)
		{
			IF_CONT(vLines[i].empty());

			// p cnf <ISING_VARs> <ISING_TERMs>
			vector<string> vL = splitBy(vLines[i], ' ');
			IF_CONT(vL[0] == "c");
			IF_CONT(vL[0] != "p");

			IF_F(vL[1] != "cnf");

			m_nV = atoi(vL[2].c_str());
			nT = atoi(vL[3].c_str());
			break;
		}

		IF_F((m_nV <= 0) || (nT <= 0));

		m_nV++; // need to +1 for const term

		// Ising terms
		ISING_JW Jw;
		Jw.init(m_nV);

		for (i++; i < vLines.size(); i++)
		{
			IF_CONT(vLines[i].empty());

			vector<string> vL = splitBy(vLines[i], ' ');
			IF_CONT(vL[0] == "c");
			IF_CONT(vL[0] == "p");

			Jw.clear();
			Jw.m_J = 1; // TODO: read Jij
			int iL = 0;
			while (vL[iL] != "0")
			{
				int s = atoi(vL[iL++].c_str());
				IF_Le_F(s >= m_nV - 1, "Spin exceeds max, line: " + i2str(i));
				Jw.addSpin(s);
			}
			IF_CONT(iL <= 0);

			m_vJw.push_back(Jw);
		}

		return true;
	}

	double _IsingBase::energy(void)
	{
		double e = 0;
		for (size_t i = 0; i < m_vJw.size(); i++)
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
		for (size_t i = 1; i < m_spinAssign.nBits(); i++)
		{
			int b = m_spinAssign.get(i);
			s += b ? "1 " : "-1 ";
		}

		LOG_I("Spin assign: " + s);
	}

	bool _IsingBase::addJw(ISING_JW *pJw, bool bMerge)
	{
		NULL_F(pJw);

		if (!bMerge)
		{
			m_vJw.push_back(*pJw);
			return true;
		}

		// merge with existing Jw if existed
		for (size_t i = 0; i < m_vJw.size(); i++)
		{
			ISING_JW *pJwi = &m_vJw[i];
			IF_CONT(pJwi->m_w != pJw->m_w);

			pJwi->m_J += pJw->m_J;
			return true;
		}

		m_vJw.push_back(*pJw);
		return true;
	}

	void _IsingBase::mergeJw(void)
	{
		for (size_t i = 0; i < m_vJw.size(); i++)
		{
			ISING_JW *pJwi = &m_vJw[i];

			for (int j = i - 1; j >= 0; j--)
			{
				ISING_JW *pJwj = &m_vJw[j];
				IF_CONT(pJwi->m_w != pJwj->m_w);

				pJwj->m_J += pJwi->m_J;
				m_vJw.erase(m_vJw.begin() + i);
				break;
			}
		}
	}

	void _IsingBase::sortJw(void)
	{
		sort(m_vJw.begin(), m_vJw.end());
	}

	ISING_JW *_IsingBase::getJw(const vBit &vB)
	{
		for (size_t i = 0; i < m_vJw.size(); i++)
		{
			ISING_JW *pJw = &m_vJw[i];
			IF_CONT(pJw->m_w != vB);

			return pJw;
		}

		return nullptr;
	}

	void _IsingBase::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);
	}

}
