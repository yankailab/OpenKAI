/*
 * _ActuatorSync.h
 *
 *  Created on: May 16, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Actuator__ActuatorSync_H_
#define OpenKAI_src_Actuator__ActuatorSync_H_

#include "_ActuatorBase.h"

#define AS_N_ACTUATOR 128

namespace kai
{

class _ActuatorSync: public _ActuatorBase
{
public:
	_ActuatorSync();
	~_ActuatorSync();

	virtual bool init(void* pKiss);
	virtual bool start(void);
	virtual int check(void);

	virtual void move(vFloat4& vSpeed);
	virtual void pos(vFloat4& vPos, vFloat4& vSpeed);
	virtual void moveToOrigin(void);

	virtual bool bComplete(void);

private:
	virtual void updateSync(void);
	virtual void update(void);
	static void* getUpdateThread(void* This)
	{
		((_ActuatorSync*) This)->update();
		return NULL;
	}

public:
	_ActuatorBase* m_pAB[AS_N_ACTUATOR];
	int m_nAB;
	int m_iABget;

};

}
#endif

