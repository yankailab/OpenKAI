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
		m_probIsing = "";
		m_nSpin = 0;
		m_vSpinAssign.clear();
		m_vJw.clear();
	}

	bool _IsingBase::readIsing(const string &fName, string *pIsing)
	{
		NULL_F(pIsing);

		IF_F(!readFile(fName, pIsing, "\n"));
		IF_F(pIsing->empty());

		return true;
	}

	bool _IsingBase::decodeIsing(const string &probIsing)
	{
		IF_F(probIsing.empty());

		// probIsing file input
		vector<string> vLines = splitBy(probIsing, '\n');

		size_t i;

		// find header
		int nT;
		for (i = 0; i < vLines.size(); i++)
		{
			IF_CONT(vLines[i].empty());

			// header format
			// c comment
			// p Ising <Ising spin number> <Ising term number>
			vector<string> vL = splitBy(vLines[i], ' ');
			IF_CONT(vL[0] == "c");
			IF_CONT(vL[0] != "p");

			IF_F(vL[1] != "Ising");

			m_nSpin = atoi(vL[2].c_str());
			nT = atoi(vL[3].c_str());
			break;
		}

		IF_F((m_nSpin <= 0) || (nT <= 0));

		// Ising terms
		ISING_JW Jw;

		for (i++; i < vLines.size(); i++)
		{
			IF_CONT(vLines[i].empty());

			// line format
			// <Ising interaction J> <Ising interaction spin indices> 0
			// Lack of <Ising interaction J> term means DC term in Ising

			vector<string> vL = splitBy(vLines[i], ' ');
			IF_CONT(vL.size() < 2);
			IF_CONT(vL[0] == "c");
			IF_CONT(vL[0] == "p");

			Jw.clear();
			Jw.m_J = atoll(vL[0].c_str());
			int iL = 1;
			while (vL[iL] != "0")
			{
				int s = atoi(vL[iL++].c_str());
				IF_Le_F(s > m_nSpin, "Spin exceeds max index, line: " + i2str(i));
				Jw.addSpin(s);

				if (iL >= vL.size())
					break;
			}

			// TODO: check for existing terms in m_vJw, IF_Le_F() to show error and return false if re-defined

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
			vLbit *pWb = &pJw->m_w;

			int jwS = 1; // TODO:

			e += pJw->m_J * jwS;
		}

		LOG_I("Energy: " + lf2str(e));
		return e;
	}

	void _IsingBase::printSolution(void)
	{
		string s = "";
		for (size_t i = 1; i < m_nSpin; i++)
		{
			int b = m_vSpinAssign[i];
			s += b ? "1 " : "-1 ";
		}

		LOG_I("Spin assign: " + s);
	}

	void _IsingBase::addJw(const ISING_JW &Jw, vector<ISING_JW> &vJw)
	{
		for (ISING_JW Jwi : vJw)
		{
			IF_CONT(Jw.m_w != Jwi.m_w);

			Jwi.m_J += Jw.m_J;
			return;
		}

		vJw.push_back(Jw);
	}

	void _IsingBase::sortJw(vector<ISING_JW> &vJw)
	{
		sort(vJw.begin(), vJw.end());
	}

	ISING_JW *_IsingBase::getJw(vector<ISING_JW> &vJw, const vLbit &vB)
	{
		for (size_t i = 0; i < vJw.size(); i++)
		{
			ISING_JW *pJw = &vJw[i];
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
