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
#include "../Sensor/_DistSensorBase.h"

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
	_DistSensorBase* m_pDS;

	vDouble3 m_vWP;		//lat, lon, alt(m)
	vDouble3 m_vPos;	//lat, lon, alt(m)
	vDouble3 m_vErr;	//lat, lon, alt(m)
	double	 m_speedV;
	double	 m_speedH;
	double	 m_hdg;
	double	 m_r;		//radius(m)

};

}
#endif
