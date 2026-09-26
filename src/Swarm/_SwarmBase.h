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

		virtual bool loadConfig(void) override;
		bool saveConfig(bool bExport) override;
		virtual bool link(void) override;
		virtual bool check(void);
		virtual void console(void *pConsole);


	protected:
	};

}
#endif
