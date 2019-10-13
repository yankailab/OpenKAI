#ifndef OpenKAI_src_Autopilot_APcopter__APcopter_WP_H_
#define OpenKAI_src_Autopilot_APcopter__APcopter_WP_H_

#include "../../../Base/common.h"
#include "../../../Sensor/_DistSensorBase.h"
#include "../../Mission/Waypoint.h"
#include "../../_AutopilotBase.h"
#include "_APcopter_base.h"

namespace kai
{

class _APcopter_WP: public _AutopilotBase
{
public:
	_APcopter_WP();
	~_APcopter_WP();

	virtual	bool init(void* pKiss);
	virtual void draw(void);
	virtual int check(void);
	virtual void update(void);

public:
	_APcopter_base* 	m_pAP;
	_DistSensorBase* m_pDS;

	double	m_dZdefault;
	double	m_kZsensor;

	AP_MOUNT m_apMount;

};

}
#endif
