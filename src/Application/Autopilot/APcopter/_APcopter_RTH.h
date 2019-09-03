#ifndef OpenKAI_src_Autopilot_APcopter__APcopter_RTH_H_
#define OpenKAI_src_Autopilot_APcopter__APcopter_RTH_H_

#include "../../../Base/common.h"
#include "../../../Sensor/_DistSensorBase.h"
#include "../../Mission/RTH.h"
#include "../../_ActionBase.h"
#include "_APcopter_base.h"

namespace kai
{

class _APcopter_RTH: public _ActionBase
{
public:
	_APcopter_RTH();
	~_APcopter_RTH();

	virtual	bool init(void* pKiss);
	virtual bool draw(void);
	virtual bool console(int& iY);
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
