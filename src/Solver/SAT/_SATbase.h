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
		int m_iV = 0;

		void assign(int iV)
		{
			m_iV = iV;
		}

		int v(void)
		{
			return m_iV;
		}
	};

	struct CLAUSE
	{
		int m_pL[N_MAX_LITERAL];
		int m_nL = 0;

		bool literal(int i, int *piV)
		{
			int L = m_pL[i];
			if (L >= 0)
			{
				*piV = L - 1;
				return true;
			}

			*piV = abs(L) - 1;
			return false;
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

		bool decodeCNF(const string& cnf);
		bool verify(void);
		void printSolution(void);

	protected:

	protected:
		// variables
		VARIABLE *m_pV = NULL;
		int m_nV;

		// clauses
		CLAUSE *m_pC = NULL;
		int m_nC;

	};

}
#endif
