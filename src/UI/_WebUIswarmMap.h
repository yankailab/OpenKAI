/*
 * _WebUIswarmMap.h
 *
 *  Created on: July 10, 2023
 *      Author: Kai Yan
 */

#ifndef OpenKAI_src_UI_WebUIswarmMap_H_
#define OpenKAI_src_UI_WebUIswarmMap_H_

#include "_WebUIbase.h"
#include "../State/_StateControl.h"
#include "../Autopilot/ArduPilot/_AP_base.h"

namespace kai
{
	struct CB_WebUIswarmMap
	{
		size_t m_wd;
		void* m_pI;
	};

	static int g_nWebUIswarmMap = 0;
	static CB_WebUIswarmMap g_cbWebUIswarmMap[128];

	class _WebUIswarmMap : public _WebUIbase
	{
	public:
		_WebUIswarmMap();
		virtual ~_WebUIswarmMap();

		virtual bool init(void *pKiss);
		virtual bool link(void);
		virtual bool start(void);
		virtual int check(void);

	protected:
		virtual void updateVehicles(void);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_WebUIswarmMap *)This)->update();
			return NULL;
		}

		virtual void show(void);
		static void *getShow(void *This)
		{
			((_WebUIswarmMap *)This)->show();
			return NULL;
		}

		virtual void btn(webui_event_t* e);
		static void cbBtn(webui_event_t* e)
		{
			for(int i=0; i<g_nWebUIswarmMap; i++)
			{
				IF_CONT(g_cbWebUIswarmMap[i].m_wd != e->window);

				((_WebUIswarmMap*)g_cbWebUIswarmMap[i].m_pI)->btn(e);
			}
		}

	private:
		vector<_AP_base*> m_vpAP;
		_StateControl *m_pSC;

	};
}
#endif
