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
	virtual int check(void);
	virtual bool update(void);
	virtual bool draw(void);
	virtual bool cli(int& iY);

	virtual int	iNextMission(void);

public:
	string m_nextMission;
	int	   m_iNextMission;

};

}
#endif
