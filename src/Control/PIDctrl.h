/*
 * PIDctrl.h
 *
 *  Created on: May 18, 2018
 *      Author: yankai
 */

#ifndef OpenKAI_src_Control_PIDctrl_H_
#define OpenKAI_src_Control_PIDctrl_H_

#include "ControlBase.h"

namespace kai
{

class PIDctrl: public ControlBase
{
public:
	PIDctrl();
	virtual ~PIDctrl();

	virtual bool init(void* pKiss);
	virtual double update(double v, double vSet, uint64_t t);
	virtual double o(void);
	virtual bool draw(void);
	virtual bool console(int& iY);
	virtual void reset(void);

public:
	double m_P;
	double m_I;
	double m_Imax;
	double m_D;

	double m_v;
	double m_vPred;
	double m_vTarget;
	double m_vMin;
	double m_vMax;

	double m_e;
	double m_eOld;
	double m_eInteg;

	double m_dT;
	double m_K;
	double m_oMin;
	double m_oMax;
	double m_output;

	uint64_t m_tLastUpdate;

};

}
#endif
