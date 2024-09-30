#ifndef OpenKAI_src_Autopilot_APmavlink__APmavlink_follow_H_
#define OpenKAI_src_Autopilot_APmavlink__APmavlink_follow_H_

#include "../../Universe/_Universe.h"
#include "../../Tracker/_TrackerBase.h"
#include "../../Filter/Median.h"
#include "../../Filter/Average.h"
#include "../../Filter/Predict.h"
#include "../../Control/PID.h"
#include "_APmavlink_move.h"

namespace kai
{

	struct FOLLOW_TARGET_FILT
	{
		Median<float> m_med;
		Predict<float> m_pred;

		bool init(int nWmed, int nWpred)
		{
			IF_F(!m_med.init(nWmed));
			IF_F(!m_pred.init(nWpred));

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

		virtual int init(void *pKiss);
		virtual int link(void);
		virtual int check(void);
		virtual int start(void);
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
		_Universe *m_pU;
		_TrackerBase *m_pTracker;
		int m_iClass;
		bool m_bTarget;
		vFloat4 m_vTargetBB;

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
		uint64_t m_tLastPIDupdate;

		PID *m_pPitch;
		PID *m_pRoll;
		PID *m_pAlt;
		PID *m_pYaw;
		
		// Gimbal mount
		AP_MOUNT m_apMount;
	};

}
#endif
