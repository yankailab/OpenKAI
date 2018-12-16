/*
 * MissionBase.h
 *
 *  Created on: Nov 13, 2018
 *      Author: yankai
 */

#ifndef OpenKAI_src_Mission_MissionBase_H_
#define OpenKAI_src_Mission_MissionBase_H_

#include "../Base/common.h"
#include "../Base/_ThreadBase.h"

namespace kai
{

class MissionBase: public BASE
{
public:
	MissionBase();
	virtual ~MissionBase();

	virtual bool init(void* pKiss);
	virtual void missionStart(void);
	virtual int check(void);
	virtual bool update(void);
	virtual bool draw(void);
	virtual bool console(int& iY);

public:
	string m_nextMission;

};

}
#endif
