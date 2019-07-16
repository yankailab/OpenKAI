/*
 * _MissionControl.h
 *
 *  Created on: Aug 27, 2016
 *      Author: Kai
 */

#ifndef OpenKAI_src_Mission__MissionControl_H_
#define OpenKAI_src_Mission__MissionControl_H_

#include "../../../Base/common.h"
#include "../../../Base/_ThreadBase.h"
#include "MissionBase.h"
#include "Waypoint.h"
#include "Land.h"
#include "Loiter.h"
#include "Move.h"
#include "RTH.h"

#define ADD_MISSION(x) if(pKM->m_class==#x){M.m_pInst=new x();M.m_pKiss=pKM;}

namespace kai
{

struct MISSION_INST
{
	MissionBase* m_pInst;
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
	bool draw(void);
	bool console(int& iY);

	MissionBase* getCurrentMission(void);
	string getCurrentMissionName(void);
	int getCurrentMissionIdx(void);
	int getLastMissionIdx(void);
	int getMissionIdx(const string& missionName);
	void transit(const string& nextMissionName);
	void transit(int iNextMission);

private:
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_MissionControl *) This)->update();
		return NULL;
	}

	vector<MISSION_INST> m_vMission;
	int		m_iMission;

};

}
#endif
