/*
 * _WebUIbase.h
 *
 *  Created on: July 10, 2023
 *      Author: Kai Yan
 */

#ifndef OpenKAI_src_UI_WebUIbase_H_
#define OpenKAI_src_UI_WebUIbase_H_

#include "../Base/_ModuleBase.h"

namespace kai
{
	class _WebUIbase : public _ModuleBase
	{
	public:
		_WebUIbase();
		virtual ~_WebUIbase();

		virtual bool init(void *pKiss);
		virtual bool link(void);
		virtual bool start(void);

	protected:
		virtual void show(void);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_WebUIbase *)This)->update();
			return NULL;
		}

		static void cbEvent(webui_event_t* e)
		{
		    webui_run(e->window, "setHTML(id,'')");
		}

	protected:
		vector<BASE *> m_vpB;
		_Thread *m_pTui;

		string m_rootDir;
		string m_fHtml;
		size_t m_wd;

		bool m_bMultiAccess;

	};
}
#endif
