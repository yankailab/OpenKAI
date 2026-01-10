/*
 * _SATbase.h
 *
 *  Created on: Oct 10, 2025
 *      Author: yankai
 */

#ifndef OpenKAI_src_Solver_SAT__SATbase_H_
#define OpenKAI_src_Solver_SAT__SATbase_H_

#include "../../Base/_ModuleBase.h"
#include "../../Primitive/tSwap.h"
#include "../../Utility/utilFile.h"

namespace kai
{
	struct VARIABLE
	{
		int m_v = 0;	//0: undefined, +/-1: true/false

		void assign(int v)
		{
			m_v = v;
		}

		int v(void)
		{
			return m_v;
		}
	};

	struct CLAUSE
	{
		vector<int> m_vL;	// var index from 1, +/- sign = negation

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

	class _SATbase : public _ModuleBase
	{
	public:
		_SATbase();
		virtual ~_SATbase();

		virtual int init(void *pKiss);
		virtual int link(void);
		virtual int check(void);
		virtual void console(void *pConsole);

		void clear(void);
		bool readCNF(const string& fName, string* pCNF);
		bool decodeCNF(const string& cnf);
		bool bSatisfied(void);
		void printSolution(void);

	protected:
		string m_fName;
		string m_cnf; // problem input

		vector<VARIABLE> m_vV;	// variable index from 1, [0] is not used
		vector<CLAUSE> m_vC;	// clauses

	};

}
#endif
