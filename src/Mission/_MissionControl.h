/*
 * _MissionControl.h
 *
 *  Created on: Aug 27, 2016
 *      Author: Kai
 */

#ifndef OpenKAI_src_Mission__MissionControl_H_
#define OpenKAI_src_Mission__MissionControl_H_

#include "../Base/common.h"
#include "../Base/_ThreadBase.h"

#define N_STATE 32

namespace kai
{

class _MissionControl: public BASE
{
public:
	_MissionControl();
	virtual ~_MissionControl();

	bool init(void* pKiss);
	bool draw(void);

	int getStateIdx(const string& stateName);
	int getCurrentStateIdx(void);
	string* getCurrentStateName(void);
	int getLastStateIdx(void);
	bool transit(const string& nextStateName);
	bool transit(int nextStateIdx);

public:
	int		m_nState;
	int		m_iState;
	string	m_pStateName[N_STATE];
	int		m_iLastState;

};

}
#endif
