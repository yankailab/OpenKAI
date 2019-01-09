/*
 * ControlBase.h
 *
 *  Created on: May 18, 2015
 *      Author: yankai
 */

#ifndef OpenKAI_src_Control_ControlBase_H_
#define OpenKAI_src_Control_ControlBase_H_

#include "../Base/common.h"
#include "../Base/BASE.h"
#include "../Script/Kiss.h"

namespace kai
{

class ControlBase: public BASE
{
public:
	ControlBase();
	virtual ~ControlBase();

	virtual bool init(void* pKiss);
	virtual void update(void);
	virtual bool draw(void);
	virtual bool console(int& iY);
	virtual void reset(void);

public:

};

}

#endif
