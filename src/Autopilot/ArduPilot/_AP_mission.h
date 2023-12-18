#ifndef OpenKAI_src_Autopilot_AP__AP_mission_H_
#define OpenKAI_src_Autopilot_AP__AP_mission_H_

#include "_AP_base.h"

namespace kai
{

	class _AP_mission : public _StateBase
	{
	public:
		_AP_mission();
		~_AP_mission();

		virtual bool init(void *pKiss);
		virtual bool link(void);
		virtual bool start(void);
		virtual int check(void);
		virtual void console(void *pConsole);

	protected:
		void updateMission(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_AP_mission *)This)->update();
			return NULL;
		}

	protected:
		_AP_base *m_pAP;
	};

}
#endif
