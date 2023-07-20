/*
 * _WebUIswarmList.h
 *
 *  Created on: July 10, 2023
 *      Author: Kai Yan
 */

#ifndef OpenKAI_src_UI_WebUIswarmList_H_
#define OpenKAI_src_UI_WebUIswarmList_H_

#include "_WebUIbase.h"
#include "../State/_StateControl.h"
#include "../Autopilot/ArduPilot/_AP_base.h"

namespace kai
{
	struct CB_WebUIswarmList
	{
		size_t m_wd;
		void* m_pI;
	};

	static int g_nWebUIswarmList = 0;
	static CB_WebUIswarmList g_cbWebUIswarmList[128];

	class _WebUIswarmList : public _WebUIbase
	{
	public:
		_WebUIswarmList();
		virtual ~_WebUIswarmList();

		virtual bool init(void *pKiss);
		virtual bool link(void);
		virtual bool start(void);
		virtual int check(void);

	protected:
		virtual void updateVehicles(void);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_WebUIswarmList *)This)->update();
			return NULL;
		}

		virtual void show(void);
		static void *getShow(void *This)
		{
			((_WebUIswarmList *)This)->show();
			return NULL;
		}

		virtual void btn(webui_event_t* e);
		static void cbBtn(webui_event_t* e)
		{
			for(int i=0; i<g_nWebUIswarmList; i++)
			{
				IF_CONT(g_cbWebUIswarmList[i].m_wd != e->window);

				((_WebUIswarmList*)g_cbWebUIswarmList[i].m_pI)->btn(e);
			}
		}

	private:
		vector<_AP_base*> m_vpAP;
		_StateControl *m_pSC;

	};
}
#endif
