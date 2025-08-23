/*
 * VisionBase.h
 *
 *  Created on: Jul 11, 2025
 *      Author: yankai
 */

#ifndef OpenKAI_src_QC__SATbase_H_
#define OpenKAI_src_QC__SATbase_H_

#include "../../Base/_ModuleBase.h"

namespace kai
{

	class _SATbase : public _ModuleBase
	{
	public:
		_SATbase();
		virtual ~_SATbase();

		virtual int init(void *pKiss);
		virtual int link(void);
		virtual int check(void);
		virtual void console(void *pConsole);

	protected:
	};

}
#endif
