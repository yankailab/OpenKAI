/*
 * RTH.h
 *
 *  Created on: Jan 14, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Mission_RTH_H_
#define OpenKAI_src_Mission_RTH_H_

#include "Mission.h"

namespace kai
{

class RTH: public Mission
{
public:
	RTH();
	virtual ~RTH();

	bool init(void* pKiss);
	bool update(void);
	void draw(void);

	void reset(void);
	void setHome(vDouble3& p);
	void setPos(vDouble3& p);
	vDouble3 getHome(void);
	double getHdg(void);

private:
	bool	m_bSetHome;

	vDouble3 m_vHome;	//lat, lon, alt
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
