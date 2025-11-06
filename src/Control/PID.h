/*
 * PIDctrl.h
 *
 *  Created on: May 18, 2018
 *      Author: yankai
 */

#ifndef OpenKAI_src_Control_PID_H_
#define OpenKAI_src_Control_PID_H_

#include "../Base/BASE.h"
#include "../Module/Kiss.h"
#include "../UI/_Console.h"

namespace kai
{

	class PID : public BASE
	{
	public:
		PID();
		virtual ~PID();

		virtual int init(void *pKiss);
		virtual float update(float v, float sp, float dT);
		virtual double o(void);
		virtual void console(void *pConsole);
		virtual void reset(void);

		float getP(void);
		void setP(float P);
		float getI(void);
		void setI(float I);
		float getImax(void);
		void setImax(float Imax);
		float getD(void);
		void setD(float D);

		vFloat2 getRangeIn(void);
		void setRangeIn(const vFloat2& vRin);

		vFloat2 getRangeOut(void);
		void setRangeOut(const vFloat2& vRout);

	protected:
		float m_P;
		float m_I;
		float m_Imax;
		float m_D;

		float m_vVar;
		float m_vSetPoint;
		float m_e;
		float m_eOld;
		float m_eI; //e integration

		float m_vOut;
		vFloat2 m_vRin;
		vFloat2 m_vRout;
	};

}
#endif
