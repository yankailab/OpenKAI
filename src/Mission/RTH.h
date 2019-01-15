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
	bool update(void);
	bool draw(void);
	bool console(int& iY);

	void reset(void);
	void setHome(vDouble3& p);
	void setPos(vDouble3& p);

private:
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
