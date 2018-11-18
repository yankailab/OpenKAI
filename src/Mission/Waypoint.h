/*
 * Waypoint.h
 *
 *  Created on: Nov 13, 2018
 *      Author: yankai
 */

#ifndef OpenKAI_src_Mission_Waypoint_H_
#define OpenKAI_src_Mission_Waypoint_H_

#include "MissionBase.h"
#include "../Protocol/_Mavlink.h"

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

public:
	_Mavlink* m_pMavlink;

	vDouble3 m_vWP;		//lat, lon, alt(m)
	vDouble3 m_vD;		//lat, lon, alt(m)
	vDouble3 m_vSpeed;	//vertical(m/s), horizontal(m/s)
	double	 m_yaw;
	double	 m_yawSpeed;
	double	 m_r;		//radius(m)


};

}
#endif
