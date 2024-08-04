#ifndef OpenKAI_src_Autopilot_MavAP__MavAP_landingTarget_H_
#define OpenKAI_src_Autopilot_MavAP__MavAP_landingTarget_H_

#include "../../Universe/_Universe.h"
#include "../../Sensor/Distance/_DistSensorBase.h"
#include "../../Utility/util.h"
#include "_MavAP_move.h"

namespace kai
{
	struct AP_LANDING_TARGET_TAG
	{
		int m_id = -1;
		int m_priority = 0;
	};

	class _MavAP_landingTarget : public _MavAP_move
	{
	public:
		_MavAP_landingTarget();
		~_MavAP_landingTarget();

		virtual int init(void *pKiss);
		virtual int link(void);
		virtual int start(void);
		virtual int check(void);
		virtual void update(void);
		virtual void console(void *pConsole);

	protected:
		virtual AP_LANDING_TARGET_TAG* getTag(int id);
		virtual bool findTag(void);
		virtual void updateLandingTarget(void);
		static void *getUpdate(void *This)
		{
			((_MavAP_landingTarget *)This)->update();
			return NULL;
		}

	protected:
		_DistSensorBase* m_pDS;
		_Universe *m_pU;
		_Object m_oTarget;

		float m_yawRate;
		float m_kP;
		float m_defaultDtgt;
		vector<AP_LANDING_TARGET_TAG> m_vTags;
		vFloat2 m_vFov; // cam FOV horiz/vert
		vFloat2 m_vPsp; // target set point on screen coord
		mavlink_landing_target_t m_lt;

		// heading adjustment
		bool m_bHdg;
		bool m_bHdgMoving;
		float m_hdgSp;
		float m_hdgDz;
		float m_hdgDzNav;
		float m_dHdg;
		float m_hTouchdown;
	};
}
#endif
