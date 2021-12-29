/*
 * ScienceBase.h
 *
 *  Created on: Aug 22, 2015
 *      Author: yankai
 */

#ifndef OpenKAI_src_Science__ScienceBase_H_
#define OpenKAI_src_Science__ScienceBase_H_

#include "../Base/_ModuleBase.h"
#include "../UI/_Console.h"
#include "../UI/_WindowCV.h"

namespace kai
{

	class _ScienceBase : public _ModuleBase
	{
	public:
		_ScienceBase();
		virtual ~_ScienceBase();

		virtual bool init(void *pKiss);
		virtual void cvDraw(void *pWindow);

	protected:
		static int cbGraph(mglGraph *gr);	
	};

}
#endif
