/*
 * RTH.h
 *
 *  Created on: Jan 14, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Mission_RTH_H_
#define OpenKAI_src_Mission_RTH_H_

#include "MissionBase.h"

namespace kai
{

class RTH: public MissionBase
{
public:
	RTH();
	virtual ~RTH();

	bool init(void* pKiss);
	bool missionStart(void);
	bool update(void);
	void reset(void);
	bool draw(void);
	bool console(int& iY);

	void setHome(vDouble3& p);
	void setPos(vDouble3& p);

public:
	bool	m_bHome;
	bool	m_bPos;

	vDouble3 m_vHome;		//lat, lon, alt
	vDouble3 m_vPos;
	vDouble3 m_vErr;

	double	 m_speedV;
	double	 m_speedH;
	double	 m_hdg;
	double	 m_r;		//radius(m)
};

}
#endif
