/*
 * Takeoff.h
 *
 *  Created on: Mar 9, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_Mission_Takeoff_H_
#define OpenKAI_src_Mission_Takeoff_H_

#include "MissionBase.h"

namespace kai
{

class Takeoff: public MissionBase
{
public:
	Takeoff();
	virtual ~Takeoff();

	bool init(void* pKiss);
	bool update(void);
	void draw(void);

	void setTakeoffed(bool bTakeoffed);

public:
	float m_alt;

};

}
#endif
