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
#include "../Primitive/vBit.h"
#include "../Utility/utilFile.h"

namespace kai
{
	struct ISING_JW
	{
		int m_J;
		vBit m_w; // spin indices bit mask

		void init(int nBits, int J = 0)
		{
			m_J = J;
			m_w.init(nBits);
		}

		void clear(void)
		{
			m_w.clearAll();
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

	protected:
		bool addJw(ISING_JW *pJw, bool bMerge = true);
		void mergeJw(void); // merge the identical Jij spin terms
		void sortJw(void);
		ISING_JW *getJw(const vBit &vB);

	protected:
		string m_fName;
		string m_cnf; // problem input

		int m_nV;		   // real spin number + 1
		vBit m_spinAssign; // bit 0 is for constant terms, bit 1~ for spin n, 0/1 to -1/1
		vector<ISING_JW> m_vJw;
	};

}
#endif
