#ifndef OpenKAI_src_Autopilot_AP__AP_landingTarget_H_
#define OpenKAI_src_Autopilot_AP__AP_landingTarget_H_

#include "../../Universe/_Universe.h"
#include "_AP_base.h"

namespace kai
{
	struct AP_LANDING_TARGET_TAG
	{
		int m_id = -1;
		int m_priority = 0;
	};

	class _AP_landingTarget : public _ModuleBase
	{
	public:
		_AP_landingTarget();
		~_AP_landingTarget();

		virtual bool init(void *pKiss);
		virtual bool link(void);
		virtual bool start(void);
		virtual int check(void);
		virtual void update(void);

	protected:
		virtual AP_LANDING_TARGET_TAG* getTag(int id);
		virtual bool findTag(void);
		virtual void updateLandingTarget(void);
		static void *getUpdate(void *This)
		{
			((_AP_landingTarget *)This)->update();
			return NULL;
		}

	protected:
		_AP_base* m_pAP;
		_Universe *m_pU;
		_Object m_oTarget;

		vector<AP_LANDING_TARGET_TAG> m_vTags;
		vFloat2 m_vFov; // cam FOV horiz/vert
	};
}
#endif
