/*
 * _MissionBase.h
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

class _MissionBase: public _ThreadBase
{
public:
	_MissionBase();
	virtual ~_MissionBase();

	virtual bool init(void* pKiss);
	virtual bool draw(void);
	virtual bool cli(int& iY);

public:

};

}
#endif
