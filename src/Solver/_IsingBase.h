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

namespace kai
{
	struct ISING_JW
	{
		int64_t m_J;
		vLbit m_w; // spin indices bit mask

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
		bool readCNF(const string &fName, string *pCNF);
		bool decodeCNF(const string &cnf);

		double energy(void);
		void printSolution(void);

		void addJw(const ISING_JW &Jw, vector<ISING_JW>& vJw);
		void sortJw(vector<ISING_JW>& vJw);
		ISING_JW *getJw(vector<ISING_JW>& vJw, const vLbit &vB);

	protected:
		string m_fName;
		string m_cnf;		// problem input

		int m_nV;			// real spin number + 1
		vLbit m_spinAssign;	// bit 0 is for constant terms, bit 1~ for spin n, 0/1 to -1/1
		vector<ISING_JW> m_vJw;
	};

}
#endif
