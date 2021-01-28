/*
 * _MissionControl.h
 *
 *  Created on: Aug 27, 2016
 *      Author: Kai
 */

#ifndef OpenKAI_src_Mission__MissionControl_H_
#define OpenKAI_src_Mission__MissionControl_H_

#include "../Base/_ThreadBase.h"
#include "Goto.h"
#include "Waypoint.h"
#include "Land.h"
#include "Loiter.h"
#include "Mission.h"
#include "RTH.h"
#include "Takeoff.h"

#define ADD_MISSION(x) if(pKM->m_class==#x){M.m_pInst=new x();M.m_pKiss=pKM;}

namespace kai
{

struct MISSION_INST
{
	Mission* m_pInst;
	Kiss* m_pKiss;

	void init(void)
	{
		m_pInst = NULL;
		m_pKiss = NULL;
	}
};

class _MissionControl: public _ThreadBase
{
public:
	_MissionControl();
	virtual ~_MissionControl();

	bool init(void* pKiss);
	bool start(void);
	void draw(void);

	Mission* getMission(void);
	string getMissionName(void);
	int getMissionIdx(void);
	MISSION_TYPE getMissionType(void);
	int getMissionIdxByName (const string& mName);
	void transit(void);
	void transit(const string& mName);
	void transit(int iM);

private:
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_MissionControl *) This)->update();
		return NULL;
	}

	vector<MISSION_INST> m_vMission;
	int m_iM;	//current mission

};

}
#endif
