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
	struct SWARM_NODE
	{
		uint64_t m_ID;
		vDouble2 m_vPos = {0,0};
		float	m_alt = 0;
		float	m_batt = 0;
		uint64_t m_tLastUpdate = 0;
	};

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
		int getNodeIdx(const uint64_t& ID);
		SWARM_NODE* getNode(const uint64_t& ID);
		bool addNode(const SWARM_NODE& n);
		bool updateNode(const SWARM_NODE& n);
		void deleteNode(const uint64_t& ID);
		void deleteNode(int i);

		void deleteExpiredNodes(const uint64_t& tExpire);
		SWARM_NODE* findClosestNode(vDouble2 vPos);
		int getNodesWithinRadius(const vDouble2& vPos, float r);

	protected:
		vector<SWARM_NODE> m_vNodes;

		uint64_t m_tExpire;
		
	};

}
#endif
