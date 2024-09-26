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

#define ACT_BF_STOP 0

	struct ACTUATOR_V
	{
		float m_v;
		float m_vTarget;
		float m_vErr;
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

		bool bInRange(float v)
		{
			IF_F(v < m_vRange.x);
			IF_F(v > m_vRange.y);

			return true;
		}

		void setTarget(float v)
		{
			m_vTarget = m_vRange.constrain(v);
		}

		void setNormalizedTarget(float v)
		{
			m_vTarget = m_vRange.x + constrain<float>(v, 0, 1) * m_vRange.d();
		}
	};

	enum ACTUATOR_CMD_TYPE
	{
		actCmd_standby,
		actCmd_pos,
		actCmd_spd,
		actCmd_accel,
		actCmd_brake,
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

		virtual void setPtarget(float p, bool bNormalized = false);
		virtual void setStarget(float s, bool bNormalized = false);
		virtual void setAtarget(float a, bool bNormalized = false);
		virtual void setBtarget(float b, bool bNormalized = false);
		virtual float getPtarget(void);
		virtual float getStarget(void);
		virtual float getAtarget(void);
		virtual float getBtarget(void);
		virtual float getCtarget(void);

		virtual float getP(void);
		virtual float getS(void);
		virtual float getA(void);
		virtual float getB(void);
		virtual float getC(void);

		virtual void gotoOrigin(void);
		virtual bool bComplete(void);
		virtual bool power(bool bON);
		virtual void setStop(void);

		// values read from feedback
		virtual void setP(float p);
		virtual void setS(float s);
		virtual void setA(float a);
		virtual void setB(float b);
		virtual void setC(float c);

	protected:
		virtual bool bCmdTimeout(void);
		virtual bool open(void);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_ActuatorBase *)This)->update();
			return NULL;
		}

	protected:
		float m_pOrigin;
		ACTUATOR_V m_p; // pos
		float m_sDir;	// speed direction
		ACTUATOR_V m_s; // speed
		ACTUATOR_V m_a; // accel
		ACTUATOR_V m_b; // brake
		ACTUATOR_V m_c; // current

		bool m_bPower;
		bool m_bReady;
		bool m_bFeedback;
		bool m_bMoving;

		pthread_mutex_t m_mutex;

		ACTUATOR_CMD_TYPE m_lastCmdType;
		uint64_t m_tLastCmd;
		uint64_t m_tCmdTimeout;

		_ActuatorBase *m_pParent;

		BIT_FLAG m_bf;
	};

}
#endif
