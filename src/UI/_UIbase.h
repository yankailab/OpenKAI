/*
 * _UIbase.h
 *
 *  Created on: May 24, 2022
 *      Author: Kai Yan
 */

#ifndef OpenKAI_src_UI_UIbase_H_
#define OpenKAI_src_UI_UIbase_H_

#include "../Base/_ModuleBase.h"

namespace kai
{
	class _UIbase : public _ModuleBase
	{
	public:
		_UIbase();
		virtual ~_UIbase();

		virtual bool init(const json& j);
		virtual bool link(const json& j, ModuleMgr* pM);
		virtual bool start(void);

	protected:
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_UIbase *)This)->update();
			return NULL;
		}

	protected:
		vector<BASE *> m_vpB;

	};
}
#endif
