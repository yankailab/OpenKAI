/*
 * _SwarmSearch.h
 *
 *  Created on: July 19, 2023
 *      Author: yankai
 */

#ifndef OpenKAI_src_Swarm_SwarmSearch_H_
#define OpenKAI_src_Swarm_SwarmSearch_H_

#include "_SwarmBase.h"
#include "../Navigation/GeoGrid.h"

namespace kai
{
	struct GCELL_SEARCH
	{
		int m_countCovered = 0;

	};

	class _SwarmSearch : public _SwarmBase
	{
	public:
		_SwarmSearch();
		virtual ~_SwarmSearch();

		virtual bool init(void *pKiss);
		virtual bool link(void);
	    virtual bool start(void);
		virtual int check(void);
		virtual void console(void *pConsole);

	protected:
		void updateNodes(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_SwarmSearch *)This)->update();
			return NULL;
		}

	protected:
		GeoGrid* m_pGG;
		GCELL_SEARCH* m_pGcell;
		int m_nGcell;

	};

}
#endif
