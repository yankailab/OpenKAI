/*
 * _PLC.h
 *
 *  Created on: May 26, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Application__PLC_H_
#define OpenKAI_src_Application__PLC_H_

#include "../Base/common.h"
#include "../Base/_ThreadBase.h"
#include "../Detector/_DetectorBase.h"
#include "../Actuator/_ActuatorBase.h"

namespace kai
{

struct PLC_ACTION
{
	float m_nPosX;
	float m_nPosY;
	float m_nPosZ;
	uint64_t m_dT;

	void init(void)
	{
		m_nPosX = 0.0;
		m_nPosY = 0.0;
		m_nPosZ = 0.0;
		m_dT = 0;
	}
};

class _PLC: public _ThreadBase
{
public:
	_PLC(void);
	virtual ~_PLC();

	bool init(void* pKiss);
	bool start(void);
	bool draw(void);
	bool console(int& iY);
	int check(void);

private:
	void updateAction(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_PLC *) This)->update();
		return NULL;
	}

private:
	_DetectorBase* m_pDet;

	_ActuatorBase* m_pAX;
	_ActuatorBase* m_pAY;
	_ActuatorBase* m_pAZ;

	vector<PLC_ACTION> m_vAction;
	int m_iAction;

};

}
#endif
