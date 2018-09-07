/*
 * PIDctrl.h
 *
 *  Created on: May 18, 2015
 *      Author: yankai
 */

#ifndef OpenKAI_src_Control_PIDctrl_H_
#define OpenKAI_src_Control_PIDctrl_H_

#include "ControlBase.h"

namespace kai
{

struct PID_SET
{
	double m_P;
	double m_I;
	double m_Imax;
	double m_D;
	double m_h;

	void init(void)
	{
		m_P = 0;
		m_I = 0;
		m_Imax = 0;
		m_D = 0;
		m_h = 0.0;
	}
};

class PIDctrl: public ControlBase
{
public:
	PIDctrl();
	virtual ~PIDctrl();

	virtual bool init(void* pKiss);
	virtual double update(double v, double vTarget);
	virtual double update(double v, double vTarget, double h);
	virtual double o(void);
	virtual bool draw(void);
	virtual bool cli(int& iY);
	virtual void reset(void);

public:
	PID_SET m_pid1;
	PID_SET m_pid2;
	double m_ovdH;
	double m_dP;
	double m_dI;
	double m_dImax;
	double m_dD;

	double m_v;
	double m_vPred;
	double m_vTarget;

	double m_e;
	double m_eOld;
	double m_eInteg;

	double m_min;
	double m_max;
	double m_dT;
	double m_K;
	double m_output;

	uint64_t m_tLastUpdate;

};

}
#endif
