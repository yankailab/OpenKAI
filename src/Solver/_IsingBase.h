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
#include "../Utility/utilFile.h"

namespace kai
{
	struct ISING_VAR
	{
		uint8_t m_v = 0; // +/-1, 0: undefined 

		void assign(int v)
		{
			m_v = v;
		}

		int v(void)
		{
			return m_v;
		}
	};

	struct ISING_TERM
	{
		int m_J;
		
		vector<int> m_vL; 

		void clear(void)
		{
			m_vL.clear();
		}

		void addLiteral(int L)
		{
			m_vL.push_back(L);
		}

		int getLiteral(int i)
		{
			IF__(i < 0, 0);
			IF__(i >= m_vL.size(), 0);

			return m_vL[i];
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
		bool bSatisfied(void);
		void printSolution(void);

	protected:
		string m_fName;
		string m_cnf; // problem input

		vector<ISING_VAR> m_vV; // variable index from 1, [0] is not used
		vector<ISING_TERM> m_vC;   // ISING_TERMs
	};

}
#endif
