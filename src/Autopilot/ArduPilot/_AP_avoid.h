
#ifndef OpenKAI_src_Autopilot_AP__AP_avoid_H_
#define OpenKAI_src_Autopilot_AP__AP_avoid_H_

#include "../../Detector/_DetectorBase.h"
#include "_AP_base.h"

namespace kai
{

	class _AP_avoid : public _ModuleBase
	{
	public:
		_AP_avoid();
		~_AP_avoid();

		int init(void *pKiss);
		int start(void);
		int check(void);
		void update(void);
		void draw(void *pFrame);
		void console(void *pConsole);

	private:
		void updateTarget(void);
		static void *getUpdate(void *This)
		{
			((_AP_avoid *)This)->update();
			return NULL;
		}

	private:
		_AP_base *m_pAP;
		_DetectorBase *m_pDet;
		_Mavlink *m_pMavlink;

		_Object m_obs;
	};

}
#endif
