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

	bool _IsingBase::readIsingProb(const string &fName, string *pIsing)
	{
		NULL_F(pIsing);

		IF_F(!readFile(fName, pIsing, "\n"));
		IF_F(pIsing->empty());

		return true;
	}

	bool _IsingBase::decodeIsingProb(const string &probIsing)
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
		set<vLbit> setJw;

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

			IF_Le_F(!setJw.insert(Jw.m_w).second, "Ising term re-defined, line: " + i2str(i));

			m_vJw.push_back(Jw);
		}

		return true;
	}

	double _IsingBase::energy(void)
	{
		double e = 0;
		vector<int64_t> vSpinJ(m_nSpin + 1, 0);
		for (const ISING_JW &Jw : m_vSpinAssign)
		{
			const vector<uint64_t> &vP = Jw.m_w.getVp();
			IF_CONT(vP.size() != 1);
			IF_CONT(vP[0] > (uint64_t)m_nSpin);

			vSpinJ[vP[0]] = Jw.m_J;
		}

		for (size_t i = 0; i < m_vJw.size(); i++)
		{
			ISING_JW *pJw = &m_vJw[i];
			vLbit *pWb = &pJw->m_w;

			int64_t J = pJw->m_J;
			for (uint64_t s : pWb->getVp())
			{
				if (s > (uint64_t)m_nSpin)
				{
					J = 0;
					break;
				}

				const int64_t sJ = vSpinJ[s];
				if (!sJ)
				{
					J = 0;
					break;
				}

				J *= sJ;
			}

			e += J;
		}

		LOG_I("Energy: " + lf2str(e));
		return e;
	}

	void _IsingBase::printSolution(void)
	{
		string s = "";
		s.reserve(m_vSpinAssign.size() * 4);
		for (const ISING_JW &Jw : m_vSpinAssign)
		{
			const vector<uint64_t> &vP = Jw.m_w.getVp();
			IF_CONT(vP.size() != 1);

			if (Jw.m_J < 0)
				s += "-";

			s += i2str((int)vP[0]) + " ";
		}

		LOG_I("Assign: " + s);
	}

	void _IsingBase::addJw(const ISING_JW &Jw)
	{
		for (ISING_JW Jwi : m_vJw)
		{
			IF_CONT(Jw.m_w != Jwi.m_w);

			Jwi.m_J += Jw.m_J;
			return;
		}

		m_vJw.push_back(Jw);
	}

	void _IsingBase::sortJw(void)
	{
		sort(m_vJw.begin(), m_vJw.end());
	}

	ISING_JW *_IsingBase::getJw(const vLbit &w)
	{
		for (size_t i = 0; i < m_vJw.size(); i++)
		{
			ISING_JW *pJw = &m_vJw[i];
			IF_CONT(pJw->m_w != w);

			return pJw;
		}

		return nullptr;
	}

	static inline vector<ISING_JW>::iterator lowerBoundSpinAssign(vector<ISING_JW> &vSpinAssign, const vLbit &w)
	{
		size_t first = 0;
		size_t count = vSpinAssign.size();

		while (count > 0)
		{
			const size_t step = count >> 1;
			const size_t i = first + step;

			if (vSpinAssign[i].m_w < w)
			{
				first = i + 1;
				count -= step + 1;
			}
			else
			{
				count = step;
			}
		}

		return vSpinAssign.begin() + first;
	}

	bool _IsingBase::assignSpin(const vLbit &w, int8_t s)
	{
		auto it = lowerBoundSpinAssign(m_vSpinAssign, w);
		IF_F(it != m_vSpinAssign.end() && it->m_w == w);

		ISING_JW Jw;
		Jw.m_w = w;
		Jw.m_J = s;
		m_vSpinAssign.insert(it, Jw);

		return true;
	}

	void _IsingBase::clearSpinAssign(const vLbit &w)
	{
		auto it = lowerBoundSpinAssign(m_vSpinAssign, w);
		IF_(it == m_vSpinAssign.end() || it->m_w != w);

		m_vSpinAssign.erase(it);
	}

	int8_t _IsingBase::getSpinAssign(const vLbit &w)
	{
		auto it = lowerBoundSpinAssign(m_vSpinAssign, w);
		IF__(it == m_vSpinAssign.end() || it->m_w != w, 0);

		return (int8_t)it->m_J;
	}

	void _IsingBase::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);
	}

}
