#ifndef OpenKAI_src_Autopilot_APmavlink__APmavlink_follow_H_
#define OpenKAI_src_Autopilot_APmavlink__APmavlink_follow_H_

#include "../../Universe/_Universe.h"
#include "../../Tracker/_TrackerBase.h"
#include "../../Filter/Median.h"
#include "../../Filter/Average.h"
#include "../../Filter/Predict.h"
#include "../../Control/PID.h"
#include "../../Utility/utilTime.h"
#include "_APmavlink_move.h"

namespace kai
{

	struct FOLLOW_TARGET_FILT
	{
		Median<float> m_med;
		Predict<float> m_pred;

		bool init(int nWmed, float kT)
		{
			IF_F(!m_med.init(nWmed));
			IF_F(!m_pred.init(kT));

			return true;
		}

		void reset(void)
		{
			m_med.reset();
			m_pred.reset();
		}

		float update(float v, float dT)
		{
			return m_pred.update(m_med.update(v), dT);
		}
	};

	class _APmavlink_follow : public _APmavlink_move
	{
	public:
		_APmavlink_follow();
		~_APmavlink_follow();

		virtual bool init(const json &j);
		virtual bool link(const json &j, ModuleMgr *pM);
		virtual bool check(void);
		virtual bool start(void);
		virtual void update(void);
		virtual void console(void *pConsole);
		virtual void draw(void *pFrame);

	protected:
		virtual void onPause(void);

		virtual void clearPID(void);
		virtual void updatePID(void);
		virtual bool updateTarget(void);
		virtual bool findTarget(void);
		static void *getUpdate(void *This)
		{
			((_APmavlink_follow *)This)->update();
			return NULL;
		}

	protected:
		_Universe *m_pU = nullptr;
		_TrackerBase *m_pTracker = nullptr;
		TIME_OUT m_tOutTargetNotFound;
		bool m_bTarget = false;
		vFloat4 m_vTargetBB;
		int m_iClass = -1;

		// Target detection pos filter
		FOLLOW_TARGET_FILT m_fX;
		FOLLOW_TARGET_FILT m_fY;
		FOLLOW_TARGET_FILT m_fZ;
		FOLLOW_TARGET_FILT m_fH;

		// PID control
		// make sure var and sp are in the same coordinate unit
		vFloat4 m_vPvar; // variable in pitch, roll, alt, yaw (NEDH)
		vFloat4 m_vPsp;	 // correspondent set point
		vFloat4 m_vSpd;	 // PID output that goes into speed ctrl
		uint64_t m_tLastPIDupdate = 0;

		PID *m_pPitch = nullptr;
		PID *m_pRoll = nullptr;
		PID *m_pAlt = nullptr;
		PID *m_pYaw = nullptr;

		// Gimbal mount
		AP_MOUNT m_apMount;
	};

}
#endif
