/*
 * _SwarmBase.h
 *
 *  Created on: April 3, 2023
 *      Author: yankai
 */

#ifndef OpenKAI_src_Swarm_SwarmBase_H_
#define OpenKAI_src_Swarm_SwarmBase_H_

#include "../Base/_ModuleBase.h"

namespace kai
{

	class _SwarmBase : public _ModuleBase
	{
	public:
		_SwarmBase();
		virtual ~_SwarmBase();

		virtual bool init(void *pKiss);
		virtual bool link(void);
		virtual int check(void);
		virtual void console(void *pConsole);

	protected:

		
	};

}
#endif
