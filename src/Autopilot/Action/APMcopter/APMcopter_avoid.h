
#ifndef OPENKAI_SRC_AUTOPILOT_ACTION_APMCOPTER_AVOID_H_
#define OPENKAI_SRC_AUTOPILOT_ACTION_APMCOPTER_AVOID_H_

#include "../../../Base/common.h"
#include "../../../Sensor/DistanceSensorBase.h"
#include "../../../Navigation/_Obstacle.h"
#include "../ActionBase.h"
#include "APMcopter_base.h"

#define N_DIR_MAV 38
#define N_DISTANCE_SENSOR 38

namespace kai
{

class APMcopter_avoid: public ActionBase
{
public:
	APMcopter_avoid();
	~APMcopter_avoid();

	bool init(void* pKiss);
	bool link(void);
	void update(void);
	bool draw(void);

private:
	void updateSensor(void);
	void updateMavlink(void);

	APMcopter_base* m_pAPM;
	int	m_nDS;
	DistanceSensorBase* m_pDS[N_DISTANCE_SENSOR];
	DIR_DIST m_dd[N_DIR_MAV];

};

}

#endif

