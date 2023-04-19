/*
 * _MeshBase.h
 *
 *  Created on: April 3, 2023
 *      Author: yankai
 */

#ifndef OpenKAI_src_Swarm_MeshBase_H_
#define OpenKAI_src_Swarm_MeshBase_H_

#include "../Base/_ModuleBase.h"

namespace kai
{
	struct MESH_EDGE
	{
		uint16_t m_id = 0;
		uint64_t m_tStamp = 0;
		
	};

	class _MeshBase : public _ModuleBase
	{
	public:
		_MeshBase();
		virtual ~_MeshBase();

		virtual bool init(void *pKiss);
		virtual bool link(void);
		virtual int check(void);
		virtual void console(void *pConsole);

	protected:

		
	};

}
#endif
