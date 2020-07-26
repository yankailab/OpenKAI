/*
 * _ActuatorBase.h
 *
 *  Created on: May 16, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Actuator__ActuatorBase_H_
#define OpenKAI_src_Actuator__ActuatorBase_H_

#include "../Base/_ThreadBase.h"

namespace kai
{

class _ActuatorBase: public _ThreadBase
{
public:
	_ActuatorBase();
	~_ActuatorBase();

	virtual bool init(void* pKiss);
	virtual bool start(void);
	virtual void draw(void);

	virtual void pos(vFloat4& vPos);
	virtual void speed(vFloat4& vSpeed);
	virtual void angle(vFloat4& vAngle);
	virtual void rotate(vFloat4& vRot);

	virtual void gotoOrigin(void);
	virtual void setGlobalTarget(vFloat4& t);

	virtual bool bComplete(void);
	virtual vFloat4 getPos(void);
	virtual vFloat4 getSpeed(void);

private:
	virtual bool open(void);
	virtual void update(void);
	static void* getUpdateThread(void* This)
	{
		((_ActuatorBase*) This)->update();
		return NULL;
	}

public:
	vFloat4 m_vNoriginPos;
	vFloat4 m_vNpos;
	vFloat4 m_vNtargetPos;
	vFloat4 m_vNposErr;
	vFloat4 m_vNspeed;
	vFloat4 m_vNtargetSpeed;

	vFloat4 m_vNoriginAngle;
	vFloat4 m_vNangle;
	vFloat4 m_vNtargetAngle;
	vFloat4 m_vNangleErr;
	vFloat4 m_vNrotSpeed;
	vFloat4 m_vNtargetRotSpeed;

	bool	m_bFeedback;

	_ActuatorBase* m_pParent;

};

}
#endif

