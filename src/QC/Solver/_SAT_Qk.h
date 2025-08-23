/*
 * _SAT_Qk.h
 *
 *  Created on: Feb 2, 2024
 *      Author: yankai
 */

#ifndef OpenKAI_src_QC_SAT_Qk_H_
#define OpenKAI_src_QC_SAT_Qk_H_

#include "../../Base/_ModuleBase.h"
#include "../../UI/_Console.h"

#include <stdio.h>
#include <stdint.h>
#include <complex.h>
#include <qiskit.h>

namespace kai
{
	class _SAT_Qk : public _ModuleBase
	{
	public:
		_SAT_Qk();
		virtual ~_SAT_Qk();

		virtual int init(void *pKiss);
		virtual int link(void);
		virtual int start(void);
		virtual int check(void);
		virtual void console(void *pConsole);

	private:
		void runQ(void);
		void updateSolver(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_SAT_Qk *)This)->update();
			return NULL;
		}

	protected:
		// post processing thread
		_Thread *m_pTPP;
	};

}
#endif
