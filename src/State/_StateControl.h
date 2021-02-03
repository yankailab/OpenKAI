/*
 * _StateControl.h
 *
 *  Created on: Aug 27, 2016
 *      Author: Kai
 */

#ifndef OpenKAI_src_State__StateControl_H_
#define OpenKAI_src_State__StateControl_H_

#include "../Base/_ModuleBase.h"
#include "../Mission/Goto.h"
#include "../Mission/Waypoint.h"
#include "../Mission/Land.h"
#include "../Mission/Loiter.h"
#include "../Mission/State.h"
#include "../Mission/RTH.h"
#include "../Mission/Takeoff.h"

#define ADD_STATE(x) if(pKs->m_class==#x){S.m_pInst=new x();S.m_pKiss=pKs;}

namespace kai
{

struct STATE_INST 
{
	State* m_pInst;
	Kiss* m_pKiss;

	void init(void)
	{
		m_pInst = NULL;
		m_pKiss = NULL;
	}
};

class _StateControl: public _ModuleBase
{
public:
	_StateControl();
	virtual ~_StateControl();

	bool init(void* pKiss);
	bool start(void);
	void draw(void);

	State* getState(void);
	string getStateName(void);
	int getStateIdx(void);
	STATE_TYPE getStateType(void);
	int getStateIdxByName (const string& n);
	void transit(void);
	void transit(const string& n);
	void transit(int iS);

private:
	void update(void);
	static void* getUpdate(void* This)
	{
		(( _StateControl *) This)->update();
		return NULL;
	}

	vector<STATE_INST> m_vState;
	int m_iS;	//current state

};

}
#endif
