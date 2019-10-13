#ifndef OpenKAI_src_Autopilot_APcopter__APcopter_RTH_H_
#define OpenKAI_src_Autopilot_APcopter__APcopter_RTH_H_

#include "../../../Base/common.h"
#include "../../../Sensor/_DistSensorBase.h"
#include "../../Mission/RTH.h"
#include "../../_AutopilotBase.h"
#include "_APcopter_base.h"

namespace kai
{

class _APcopter_RTH: public _AutopilotBase
{
public:
	_APcopter_RTH();
	~_APcopter_RTH();

	virtual	bool init(void* pKiss);
	virtual void draw(void);
	virtual int check(void);
	virtual void update(void);

public:
	_APcopter_base* 	m_pAP;
	_DistSensorBase* m_pDS;

	double	m_kZsensor;

	AP_MOUNT m_apMount;
};

}
#endif
