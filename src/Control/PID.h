/*
 * PIDctrl.h
 *
 *  Created on: May 18, 2018
 *      Author: yankai
 */

#ifndef OpenKAI_src_Control_PID_H_
#define OpenKAI_src_Control_PID_H_

#include "../Base/BASE.h"
#include "../Script/Kiss.h"

namespace kai
{

class PID: public BASE
{
public:
	PID();
	virtual ~PID();

	virtual bool init(void* pKiss);
	virtual float update(float v, float vT, float dT);
	virtual double o(void);
	virtual void draw(void);
	virtual void reset(void);

public:
	float m_P;
	float m_I;
	float m_Imax;
	float m_D;

	float m_v;		//input v
	float m_vT;		//target v
	float m_vMin;
	float m_vMax;

	float m_e;
	float m_eOld;
	float m_eI;		//e integration

	float m_vOut;
	float m_oMin;
	float m_oMax;
};

}
#endif
