#ifndef OpenKAI_src_Autopilot_ArduPilot__APmav_visionEstimate_H_
#define OpenKAI_src_Autopilot_ArduPilot__APmav_visionEstimate_H_

#include "../../Navigation/_NavBase.h"
#include "_APmav_base.h"

// https://ardupilot.org/copter/docs/common-vio-tracking-camera.html

namespace kai
{

	class _APmav_visionEstimate : public _ModuleBase
	{
	public:
		_APmav_visionEstimate();
		~_APmav_visionEstimate();

		virtual bool init(const json &j);
		virtual bool link(const json &j, ModuleMgr *pM);
		virtual bool start(void);
		virtual bool check(void);
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
			((_APmav_visionEstimate *)This)->update();
			return NULL;
		}

	protected:
		_APmav_base *m_pAP = nullptr;
		_NavBase *m_pNav = nullptr;

		float m_linearAccelCov = 0.01;
		float m_angularVelCov = 0.01;
		float m_conf;
		float m_covPose;
		float m_covTwist;
		Eigen::Matrix4f m_mTsensor2aero;
		Eigen::Matrix4f m_mTaero2sensor;
		float m_thrJumpPos = 0.1; // m
		float m_thrJumpSpd = 20.0; // m/s
		Vector3f m_vTprev = Vector3f::Zero();
		Vector3f m_vVprev = Vector3f::Zero();
		int m_iReset = 0;
		Vector3i m_vAxisRPY = Vector3i::Zero();

		int m_apModeInError = -1;
		bool m_bNaN = false;
		bool m_bPos = true;
		bool m_bSpd = true;
		mavlink_vision_position_estimate_t m_Dpos;
		mavlink_vision_speed_estimate_t m_Dspd;
	};

}
#endif
