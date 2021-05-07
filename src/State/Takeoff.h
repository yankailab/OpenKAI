/*
 * Takeoff.h
 *
 *  Created on: Mar 9, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_State_Takeoff_H_
#define OpenKAI_src_State_Takeoff_H_

#include "State.h"

namespace kai
{

class Takeoff: public State
{
public:
	Takeoff();
	virtual ~Takeoff();

	bool init(void* pKiss);
	bool update(void);
	void console(void* pConsole);

	void setTakeoffed(bool bTakeoffed);

public:
	float m_alt;

};

}
#endif
