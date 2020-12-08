/*
 * PIDctrl.h
 *
 *  Created on: May 18, 2018
 *      Author: yankai
 */

#ifndef OpenKAI_src_Control_PIDctrl_H_
#define OpenKAI_src_Control_PIDctrl_H_

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
	virtual float update(float v, float vTarget, uint64_t t);
	virtual double o(void);
	virtual void draw(void);
	virtual void reset(void);

public:
	float m_P;
	float m_I;
	float m_Imax;
	float m_D;

	float m_v;
	float m_vPred;
	float m_vTarget;
	float m_vMin;
	float m_vMax;

	float m_e;
	float m_eOld;
	float m_eInteg;

	float m_dT;
	float m_oMin;
	float m_oMax;
	float m_output;

	float m_A;
	float m_thrReset;
	uint64_t m_tLastUpdate;

};

}
#endif
