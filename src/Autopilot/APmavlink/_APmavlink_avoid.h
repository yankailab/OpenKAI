
#ifndef OpenKAI_src_Autopilot_APmavlink__APmavlink_avoid_H_
#define OpenKAI_src_Autopilot_APmavlink__APmavlink_avoid_H_

#include "../../Detector/_DetectorBase.h"
#include "_APmavlink_base.h"

namespace kai
{

	class _APmavlink_avoid : public _ModuleBase
	{
	public:
		_APmavlink_avoid();
		~_APmavlink_avoid();

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
			((_APmavlink_avoid *)This)->update();
			return NULL;
		}

	private:
		_APmavlink_base *m_pAP;
		_DetectorBase *m_pDet;
		_Mavlink *m_pMavlink;

		_Object m_obs;
	};

}
#endif
