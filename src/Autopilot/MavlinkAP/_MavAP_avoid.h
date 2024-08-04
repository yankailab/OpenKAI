
#ifndef OpenKAI_src_Autopilot_MavAP__MavAP_avoid_H_
#define OpenKAI_src_Autopilot_MavAP__MavAP_avoid_H_

#include "../../Detector/_DetectorBase.h"
#include "_MavAP_base.h"

namespace kai
{

	class _MavAP_avoid : public _ModuleBase
	{
	public:
		_MavAP_avoid();
		~_MavAP_avoid();

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
			((_MavAP_avoid *)This)->update();
			return NULL;
		}

	private:
		_MavAP_base *m_pAP;
		_DetectorBase *m_pDet;
		_Mavlink *m_pMavlink;

		_Object m_obs;
	};

}
#endif
