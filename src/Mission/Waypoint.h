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
#include "../Navigation/GPS.h"

namespace kai
{

class Waypoint: public MissionBase
{
public:
	Waypoint();
	virtual ~Waypoint();

	bool init(void* pKiss);
	void missionStart(void);
	int check(void);
	bool update(void);
	bool draw(void);
	bool console(int& iY);

public:
	void updatePos(void);

	_Mavlink* m_pMavlink;
	_DistSensorBase* m_pDS;
	double	m_dSensor;

	bool m_bOffset;
	bool m_bHdgOffset;
	vDouble3 m_vWP;		//lat, lon, relative_alt(m)
	vDouble3 m_vPos;
	vDouble3 m_vErr;
	double	 m_speedV;
	double	 m_speedH;
	double	 m_hdg;
	double	 m_r;		//radius(m)

};

}
#endif
