/*
 * Waypoint.h
 *
 *  Created on: Nov 13, 2018
 *      Author: yankai
 */

#ifndef OpenKAI_src_Mission_Waypoint_H_
#define OpenKAI_src_Mission_Waypoint_H_

#include "MissionBase.h"

namespace kai
{

class Waypoint: public MissionBase
{
public:
	Waypoint();
	virtual ~Waypoint();

	bool init(void* pKiss);
	int check(void);
	bool update(void);
	bool draw(void);
	bool cli(int& iY);

};

}
#endif
