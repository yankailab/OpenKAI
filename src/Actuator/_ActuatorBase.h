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
	virtual bool draw(void);
	virtual bool console(int& iY);

	virtual void move(vFloat4& vSpeed);
	virtual void moveTo(vFloat4& vPos, vFloat4& vSpeed);
	virtual void moveToOrigin(void);

	virtual void setGlobalTarget(vFloat4& t);

	virtual bool bComplete(void);
	virtual float pos(void);
	virtual float speed(void);

private:
	virtual bool open(void);
	virtual void update(void);
	static void* getUpdateThread(void* This)
	{
		((_ActuatorBase*) This)->update();
		return NULL;
	}

public:
	vFloat4 m_vNormOrigin;
	vFloat4 m_vNormPos;
	vFloat4 m_vNormTargetPos;
	vFloat4 m_vNormSpeed;
	vFloat4 m_vNormTargetSpeed;
	vFloat4 m_vNormPosErr;

	_ActuatorBase* m_pParent;

};

}
#endif

