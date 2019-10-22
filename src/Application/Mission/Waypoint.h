/*
 * Waypoint.h
 *
 *  Created on: Nov 13, 2018
 *      Author: yankai
 */

#ifndef OpenKAI_src_Mission_Waypoint_H_
#define OpenKAI_src_Mission_Waypoint_H_

#include "MissionBase.h"
#include "../../Navigation/Coordinate.h"

namespace kai
{

struct MISSION_WAYPOINT
{
	vDouble4	m_vP;	//lat, lon, alt, hdg
	vDouble4	m_vV;
	vDouble4	m_vErr;

	void init(void)
	{
		m_vP.init();
		m_vV.init();
		m_vErr.init(3.0);
	}

	bool update(vDouble4& vP, vDouble4* pE)
	{
		NULL_F(pE);
		bool b = true;

		if(vP.x >= 0.0)
		{
			pE->x = abs(vP.x - m_vP.x);
			if(pE->x > m_vErr.x)
				b = false;
		}
		else
		{
			pE->x = -1.0;
		}

		if(vP.y >= 0.0)
		{
			pE->y = abs(vP.y - m_vP.y);
			if(pE->y > m_vErr.y)
				b = false;
		}
		else
		{
			pE->y = -1.0;
		}

		if(vP.z >= 0.0)
		{
			pE->z = abs(vP.z - m_vP.z);
			if(pE->z > m_vErr.z)
				b = false;
		}
		else
		{
			pE->z = -1.0;
		}

		if(vP.w >= 0.0)
		{
			pE->w = abs(vP.w - m_vP.w);
			if(pE->w > m_vErr.w)
				b = false;
		}
		else
		{
			pE->w = -1.0;
		}

		return b;
	}

};

enum WP_LOOP
{
	wp_loop_none,
	wp_loop_repeat,
	wp_loop_shuttle,
};

class Waypoint: public MissionBase
{
public:
	Waypoint();
	virtual ~Waypoint();

	bool init(void* pKiss);
	int	 check(void);
	bool update(void);
	void reset(void);
	void draw(void);

	void setPos(vDouble4& vPos);
	MISSION_WAYPOINT* getWaypoint(void);
	MISSION_WAYPOINT* getClosestWaypoint(void);

public:
	vector<MISSION_WAYPOINT> m_vWP;
	int		m_iWP;
	int		m_dWP;
	WP_LOOP m_loop;
	vDouble4 m_vPos;
	vDouble4 m_vErr;

};

}
#endif
