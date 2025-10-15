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
		actuator_power = 0,
		actuator_setID = 1,
		actuator_setMode = 2,
		actuator_stop = 3,
		actuator_pos = 4,
		actuator_speed = 5,
		actuator_accel = 6,
		actuator_brake = 7,
	};

	enum ACTUATOR_BF_STATUS
	{
		actuator_powered = 0,
		actuator_ready = 1,
		actuator_feedbak = 2,
		actuator_moving = 3,
		actuator_complete = 4,
	};

	struct ACTUATOR_V
	{
		float m_v;
		float m_vTarget;
		float m_vCofactor;
		float m_vErr;
		vFloat2 m_vRange;

		void init(void)
		{
			m_v = 0.0;
			m_vTarget = 0.0;
			m_vCofactor = 1.0;
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
	};

	struct ACTUATOR_CHAN
	{
		int m_ID;
		float m_pOrigin;
		ACTUATOR_V m_p; // pos
		ACTUATOR_V m_s; // speed
		ACTUATOR_V m_a; // accel
		ACTUATOR_V m_b; // brake
		ACTUATOR_V m_c; // current

		int m_mode;
		bool m_bPower;
		uint64_t m_tLastCmd;
		uint64_t m_tCmdTimeout;
		BIT_FLAG m_bfStatus;
		BIT_FLAG m_bfSet;

		void init(void)
		{
			m_ID = 0;
			m_pOrigin = 0.0;
			m_p.init();
			m_s.init();
			m_a.init();
			m_b.init();
			m_c.init();

			m_mode = 0;
			m_bPower = false;
			m_tLastCmd = 0;
			m_tCmdTimeout = 0;
			m_bfStatus.clearAll();
			m_bfSet.clearAll();
		}

		int getID(void)
		{
			return m_ID;
		}

		int getMode(void)
		{
			return m_mode;
		}

		void power(bool bON)
		{
			m_bPower = bON;
			m_bfSet.set(actuator_power);
		}

		void setBitFlag(ACTUATOR_BF_SET bf)
		{
			m_bfSet.set(bf);
		}

		void setLastCmdTime(void)
		{
			m_tLastCmd = getTbootUs();
		}

		bool bCmdTimeout(void)
		{
			uint64_t t = getTbootUs();
			IF_F(t - m_tLastCmd < m_tCmdTimeout);

			return true;
		}

		void gotoOrigin(void)
		{
			m_p.setTarget(m_pOrigin);
		}

		bool bComplete(void)
		{
			IF_F(!m_p.bComplete());

			return true;
		}

		ACTUATOR_V *pos(void)
		{
			return &m_p;
		}

		ACTUATOR_V *speed(void)
		{
			return &m_s;
		}

		ACTUATOR_V *accel(void)
		{
			return &m_a;
		}

		ACTUATOR_V *brake(void)
		{
			return &m_b;
		}

		ACTUATOR_V *current(void)
		{
			return &m_c;
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

		virtual void atomicFrom(void);
		virtual void atomicTo(void);
		virtual ACTUATOR_CHAN *getChan(int iChan = 0);

	protected:
		virtual bool bCmdTimeout(int iChan = 0);
		virtual bool open(void);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_ActuatorBase *)This)->update();
			return NULL;
		}

	protected:
		vector<ACTUATOR_CHAN> m_vChan;

		_ActuatorBase *m_pParent;
		pthread_mutex_t m_mutex;
	};

}
#endif
