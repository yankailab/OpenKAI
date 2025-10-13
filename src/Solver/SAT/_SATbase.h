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

#define N_MAX_LITERAL 10

namespace kai
{
	struct VARIABLE
	{
		int m_v = -1;	//-1: undefined, 0: false, 1: true

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
		int m_pL[N_MAX_LITERAL];	// var index from 1, +/- sign = negation
		int m_nL = 0;

		void clear(void)
		{
			m_nL = 0;
		}

		bool addLiteral(int L)
		{
			IF_F(m_nL >= N_MAX_LITERAL);

			m_pL[m_nL++] = L;
			return true;
		}

		int getLiteral(int i)
		{
			IF__(i < 0, 0);
			IF__(i >= m_nL, 0);

			return m_pL[i];
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
		bool verify(void);
		void printSolution(void);

	protected:
		string m_fName;
		string m_cnf; // problem input

		// variables
		VARIABLE *m_pV;	// variable index from 1, [0] is not used
		int m_nV;	// actual number of variables + 1

		// clauses
		CLAUSE *m_pC;
		int m_nC;

	};

}
#endif
