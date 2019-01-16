#ifndef OpenKAI_src_Autopilot_Controller_APcopter_RTH_H_
#define OpenKAI_src_Autopilot_Controller_APcopter_RTH_H_

#include "../../../Base/common.h"
#include "../../../Sensor/_DistSensorBase.h"
#include "../../../Mission/RTH.h"
#include "../../ActionBase.h"
#include "APcopter_base.h"

namespace kai
{

class APcopter_RTH: public ActionBase
{
public:
	APcopter_RTH();
	~APcopter_RTH();

	virtual	bool init(void* pKiss);
	virtual bool draw(void);
	virtual bool console(int& iY);
	virtual int check(void);
	virtual void update(void);

public:
	APcopter_base* 	m_pAP;
	_DistSensorBase* m_pDS;

	double	m_kZsensor;

};

}
#endif
