/*
 * VisionBase.h
 *
 *  Created on: Jul 11, 2025
 *      Author: yankai
 */

#ifndef OpenKAI_src_QC__QCbase_H_
#define OpenKAI_src_QC__QCbase_H_

#include "../Base/_ModuleBase.h"

namespace kai
{

	class _QCbase : public _ModuleBase
	{
	public:
		_QCbase();
		virtual ~_QCbase();

		virtual int init(void *pKiss);
		virtual int link(void);
		virtual int check(void);
		virtual void console(void *pConsole);
		virtual void draw(void *pFrame);

	protected:
	};

}
#endif
