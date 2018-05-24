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

class PIDctrl: public ControlBase
{
public:
	PIDctrl();
	virtual ~PIDctrl();

	virtual bool init(void* pKiss);
	virtual bool link(void);
	virtual void update(void);
	virtual bool draw(void);
	virtual bool cli(int& iY);

	virtual void resetState(void);

public:
	double m_P;
	double m_I;
	double m_Imax;
	double m_D;
	double m_dT;

	double m_v;
	double m_vPred;
	double m_vTarget;

	double m_e;
	double m_eOld;
	double m_eInteg;

};

}
#endif
