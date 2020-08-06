/*
 * Mission.h
 *
 *  Created on: Nov 13, 2018
 *      Author: yankai
 */

#ifndef OpenKAI_src_Mission_Mission_H_
#define OpenKAI_src_Mission_Mission_H_

#include "../../Base/_ThreadBase.h"

namespace kai
{

enum MISSION_TYPE
{
	mission_unknown,
	mission_base,
	mission_land,
	mission_loiter,
	mission_goto,
	mission_rth,
	mission_wp,
	mission_takeoff,
};

class Mission: public BASE
{
public:
	Mission();
	virtual ~Mission();

	virtual bool init(void* pKiss);
	virtual bool update(void);
	virtual void reset(void);
	virtual void draw(void);
	virtual MISSION_TYPE type(void);

	virtual void complete(void);

public:
	MISSION_TYPE m_type;
	string m_next;

	uint64_t m_tStart;
	uint64_t m_tStamp;
	uint64_t m_tTimeout;
	bool m_bComplete;

};

}
#endif
