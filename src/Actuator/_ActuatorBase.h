/*
 * _ActuatorBase.h
 *
 *  Created on: May 16, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Actuator__ActuatorBase_H_
#define OpenKAI_src_Actuator__ActuatorBase_H_

#include "../Base/_ModuleBase.h"
#include "../UI/_Console.h"
#include "../Utility/BitFlag.h"

namespace kai
{

	enum ACTUATOR_BF_SET
	{
		actuator_power = 0,		 // set to energize
		actuator_setID = 1,		 // set to send set ID cmd
		actuator_setMode = 2,	 // set to send set mode cmd
		actuator_clearAlarm = 3, // set to clear alarm
		actuator_gotoOrigin = 4, // set to send goto origin command
		actuator_move = 5,		 // set to activate the move, unset to stop
		actuator_pos = 6,		 // set to send pos commands in update loop
		actuator_speed = 7,
		actuator_accel = 8,
		actuator_brake = 9,
	};

	enum ACTUATOR_BF_STATUS
	{
		actuator_powered = 0,  // set when energized
		actuator_alarm = 1,	   // set when there is an alarm
		actuator_ready = 2,	   // set when ready to move
		actuator_moving = 3,   // set during moving to target pos
		actuator_complete = 4, // achieved target pos within acceptable error range
	};

	struct ACTUATOR_V
	{
		float m_v;
		float m_vTarget;
		float m_vErr; // acceptable err range
		vFloat2 m_vRange;

		void init(void)
		{
			m_v = 0.0;
			m_vTarget = 0.0;
			m_vErr = 0.0;
			m_vRange.set(-FLT_MAX, FLT_MAX);
		}

		bool bComplete(void)
		{
			return EQUAL(m_v, m_vTarget, m_vErr);
		}

		void set(float v)
		{
			m_v = v;
		}

		float get(void)
		{
			return m_v;
		}

		void setTarget(float v)
		{
			m_vTarget = m_vRange.constrain(v);
		}

		void setTargetNormalized(float v)
		{
			m_vTarget = m_vRange.x + constrain<float>(v, 0, 1) * (m_vRange.y - m_vRange.x);
		}

		void setTargetCurrent(void)
		{
			m_vTarget = m_v;
		}

		float getTarget(void)
		{
			return m_vTarget;
		}

		float getDtarget(void)
		{
			return m_vTarget - m_v;
		}

		vFloat2 getRange(void)
		{
			return m_vRange;
		}

		void setErrRange(float v)
		{
			m_vErr = v;
		}

		float getErrRange(void)
		{
			return m_vErr;
		}
	};

	class _ActuatorBase : public _ModuleBase
	{
	public:
		_ActuatorBase();
		~_ActuatorBase();

		virtual int init(void *pKiss);
		virtual int link(void);
		virtual int start(void);
		virtual void console(void *pConsole);

		int getID(void);
		void power(bool bON);
		void move(bool bMove);
		void gotoOrigin(void);
		bool bComplete(void);
		bool bAlarm(void);

		ACTUATOR_V *pos(void);
		ACTUATOR_V *speed(void);
		ACTUATOR_V *accel(void);
		ACTUATOR_V *brake(void);
		ACTUATOR_V *current(void);

	protected:
		void setBitFlag(ACTUATOR_BF_SET bf);
		void setLastCmdTime(void);

		virtual bool bCmdTimeout(void);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_ActuatorBase *)This)->update();
			return NULL;
		}

	protected:
		int m_ID;
		float m_origin;
		ACTUATOR_V m_p; // pos
		ACTUATOR_V m_s; // speed
		ACTUATOR_V m_a; // accel
		ACTUATOR_V m_b; // brake
		ACTUATOR_V m_c; // current

		uint64_t m_tLastCmd;
		uint64_t m_tCmdTimeout;
		INTERVAL_EVENT m_ieCheckAlarm;
		INTERVAL_EVENT m_ieReadStatus;
		INTERVAL_EVENT m_ieSendCMD;

		BIT_FLAG m_bfStatus;
		BIT_FLAG m_bfSet;

		_ActuatorBase *m_pParent;
	};

}
#endif
