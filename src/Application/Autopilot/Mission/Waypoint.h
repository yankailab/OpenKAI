/*
 * Waypoint.h
 *
 *  Created on: Nov 13, 2018
 *      Author: yankai
 */

#ifndef OpenKAI_src_Mission_Waypoint_H_
#define OpenKAI_src_Mission_Waypoint_H_

#include "MissionBase.h"
#include "../../../Navigation/Coordinate.h"

namespace kai
{

class Waypoint: public MissionBase
{
public:
	Waypoint();
	virtual ~Waypoint();

	bool init(void* pKiss);
	bool update(void);
	void reset(void);
	bool draw(void);
	bool console(int& iY);

	void setPos(vDouble3& p);

public:
	bool m_bSetWP;
	bool m_bSetPos;

	bool m_bHoffset;
	bool m_bVoffset;
	bool m_bHdgOffset;

	vDouble3 m_vWPtarget;	//lat, lon, alt
	vDouble3 m_vWP;
	vDouble3 m_vPos;
	double	 m_eH;
	double	 m_eV;
	bool	 m_bAlt;

	double	 m_speedV;
	double	 m_speedH;
	double	 m_hdg;
	double	 m_r;		//radius(m)
};

}
#endif
