/*
 * _IsingBase.h
 *
 *  Created on: Feb 26, 2026
 *      Author: yankai
 */

#ifndef OpenKAI_src_Solver__IsingBase_H_
#define OpenKAI_src_Solver__IsingBase_H_

#include "../Base/_ModuleBase.h"
#include "../Primitive/tSwap.h"
#include "../Primitive/vLbit.h"
#include "../Utility/utilFile.h"
#include <set>

namespace kai
{
	struct ISING_JW
	{
		int64_t m_J;
		vLbit m_w; // Ising spin indices, empty indices as DC term

		void clear(void)
		{
			m_J = 0;
			m_w.clear();
		}

		void addSpin(int i)
		{
			m_w.set(i);
		}

		bool operator<(const ISING_JW &Jw) const
		{
			return m_w < Jw.m_w;
		}
	};

	class _IsingBase : public _ModuleBase
	{
	public:
		_IsingBase();
		virtual ~_IsingBase();

		virtual bool init(const json &j);
		virtual bool link(const json &j, ModuleMgr *pM);
		virtual bool check(void);
		virtual void console(void *pConsole);

		void clear(void);
		bool readIsingProb(const string &fName, string *pCNF);
		bool decodeIsingProb(const string &cnf);
		double energy(void);
		void printSolution(void);

		void addJw(const ISING_JW &Jw);
		void sortJw(void);
		ISING_JW *getJw(const vLbit &w);

		bool assignSpin(const vLbit &w, int8_t s);	// return true if added assignment into m_vSpinAssign, false if already assigned
		void clearSpinAssign(const vLbit &w);	// delete the element in m_vSpinAssign if existed for w
		int8_t getSpinAssign(const vLbit &w);	// return the assignment +1/-1 from m_vSpinAssign if existed, return 0 if not assigned in m_vSpinAssign
		vector<ISING_JW>::iterator findSpinAssign(const vLbit &w);

	protected:
		string m_fName;
		string m_probIsing; 			// problem input

		int m_nSpin;					// spin number
		vector<ISING_JW> m_vJw; 		// Ising spin interaction terms
		vector<ISING_JW> m_vSpinAssign;	// Ising spin assignment, m_w of single index for single spin assignment, m_J = +1/-1 as its assignment
	};

}
#endif
