/*
 * _IsingBase.h
 *
 *  Created on: Feb 26, 2026
 *      Author: yankai
 */

#ifndef OpenKAI_src_Solver_SAT__IsingBase_H_
#define OpenKAI_src_Solver_SAT__IsingBase_H_

#include "../Base/_ModuleBase.h"
#include "../Primitive/tSwap.h"
#include "../Primitive/vBit.h"
#include "../Utility/utilFile.h"

namespace kai
{
	struct ISING_JW
	{
		int m_J;
		vBit m_w;	// spin multiplication as walsh indices

		void init(int J, int nBits)
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
		string m_fName;
		string m_cnf; // problem input

		vBit m_spinAssign;
		vector<ISING_JW> m_vJw;
	};

}
#endif
