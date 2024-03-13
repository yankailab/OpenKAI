#ifndef OpenKAI_src_Autopilot_AP__AP_visionEstimate_H_
#define OpenKAI_src_Autopilot_AP__AP_visionEstimate_H_

#include "../../Navigation/_NavBase.h"
#include "_AP_base.h"
using namespace Eigen;

// https://ardupilot.org/copter/docs/common-vio-tracking-camera.html

namespace kai
{

	class _AP_visionEstimate : public _ModuleBase
	{
	public:
		_AP_visionEstimate();
		~_AP_visionEstimate();

		virtual bool init(void *pKiss);
		virtual bool link(void);
		virtual bool start(void);
		virtual int check(void);
		virtual void update(void);
		virtual void console(void *pConsole);

	protected:
		void sendPosEstimate(void);
		void sendSpeedEstimate(void);
		void updateResetCounter(void);
		bool bNaN(void);
		bool updateVisionEstimate(void);
		static void *getUpdate(void *This)
		{
			((_AP_visionEstimate *)This)->update();
			return NULL;
		}

	public:
		_AP_base *m_pAP;
		_NavBase *m_pNav;

		float m_linearAccelCov;
		float m_angularVelCov;
		float m_conf;
		float m_covPose;
		float m_covTwist;
		Matrix4f m_mTsensor2aero;
		Matrix4f m_mTaero2sensor;
		float m_thrJumpPos;
		float m_thrJumpSpd;
		vFloat3 m_vTprev;
		vFloat3 m_vVprev;
		int m_iReset;
		vInt3 m_vAxisRPY;

		int m_apModeInError;
		bool m_bNaN;
		bool m_bPos;
		bool m_bSpd;
		mavlink_vision_position_estimate_t m_Dpos;
		mavlink_vision_speed_estimate_t m_Dspd;
	};

}
#endif
