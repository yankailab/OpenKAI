/*
 * _WebUIswarm.h
 *
 *  Created on: July 10, 2023
 *      Author: Kai Yan
 */

#ifndef OpenKAI_src_UI_WebUIswarm_H_
#define OpenKAI_src_UI_WebUIswarm_H_

#include "_WebUIbase.h"
#include "../State/_StateControl.h"
#include "../Autopilot/ArduPilot/_AP_base.h"

namespace kai
{
	struct CB_WebUIswarm
	{
		size_t m_wd;
		void* m_pI;
	};

	static int g_nWebUIswarm = 0;
	static CB_WebUIswarm g_cbWebUIswarm[128];

	class _WebUIswarm : public _WebUIbase
	{
	public:
		_WebUIswarm();
		virtual ~_WebUIswarm();

		virtual bool init(void *pKiss);
		virtual bool link(void);
		virtual bool start(void);
		virtual int check(void);

	protected:
		virtual void updateVehicles(void);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_WebUIswarm *)This)->update();
			return NULL;
		}

		virtual void show(void);
		static void *getShow(void *This)
		{
			((_WebUIswarm *)This)->show();
			return NULL;
		}

		virtual void btn(webui_event_t* e);
		static void cbBtn(webui_event_t* e)
		{
			for(int i=0; i<g_nWebUIswarm; i++)
			{
				IF_CONT(g_cbWebUIswarm[i].m_wd != e->window);

				((_WebUIswarm*)g_cbWebUIswarm[i].m_pI)->btn(e);
			}
		}

	private:
		vector<_AP_base*> m_vpAP;
		_StateControl *m_pSC;

	};
}
#endif
