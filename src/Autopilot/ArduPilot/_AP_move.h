#ifndef OpenKAI_src_Autopilot_AP__AP_move_H_
#define OpenKAI_src_Autopilot_AP__AP_move_H_

#include "../../Control/PID.h"
#include "_AP_base.h"

namespace kai
{

	class _AP_move : public _StateBase
	{
	public:
		_AP_move();
		~_AP_move();

		virtual bool init(void *pKiss);
		virtual bool link(void);
		virtual bool start(void);
		virtual void update(void);
		virtual void console(void *pConsole);
		virtual int check(void);

		void clearPID(void);
		void stop(void);

		void setPosLocal(uint8_t frame = MAV_FRAME_BODY_OFFSET_NED,
						 bool bP = false,	// disable
						 bool bV = true,	// enable
						 bool bA = false,
						 bool bYaw = false,
						 bool bYawRate = true);

		void setPosGlobal(uint8_t frame = MAV_FRAME_GLOBAL_RELATIVE_ALT_INT,
						 bool bP = true,
						 bool bV = false,
						 bool bA = false,
						 bool bYaw = true,
						 bool bYawRate = false);

	private:
		static void *getUpdate(void *This)
		{
			((_AP_move *)This)->update();
			return NULL;
		}

	protected:
		_AP_base *m_pAP;

		mavlink_set_position_target_local_ned_t m_sptLocal;
		mavlink_set_position_target_global_int_t m_sptGlobal;

		// pitch, roll, alt, yaw (NEDH)
		// make sure var and sp are in the same coordinate unit
		vFloat4 m_vPvar; //variable
		vFloat4 m_vPsp;	 //correspondent set point
		vDouble4 m_vPspGlobal; //lat, lon, alt, hdg

		PID *m_pPitch;
		PID *m_pRoll;
		PID *m_pAlt;
		PID *m_pYaw;
		
		//extra factor for distance effected PID aggresiveness adjustment
		vFloat2 m_vKpidIn;
		vFloat2 m_vKpidOut;
		vFloat4 m_vKpid;
	};

}
#endif
