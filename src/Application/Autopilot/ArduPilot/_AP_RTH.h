#ifndef OpenKAI_src_Autopilot_AP__AP_RTH_H_
#define OpenKAI_src_Autopilot_AP__AP_RTH_H_

#include "../../../Base/common.h"
#include "../../../Sensor/_DistSensorBase.h"
#include "../../Mission/RTH.h"
#include "../ArduPilot/_AP_base.h"

namespace kai
{

class _AP_RTH: public _MissionBase
{
public:
	_AP_RTH();
	~_AP_RTH();

	virtual	bool init(void* pKiss);
	virtual void draw(void);
	virtual int check(void);
	virtual void update(void);

public:
	_AP_base* 	m_pAP;
	_DistSensorBase* m_pDS;

	double	m_kZsensor;

	AP_MOUNT m_apMount;
};

}
#endif
