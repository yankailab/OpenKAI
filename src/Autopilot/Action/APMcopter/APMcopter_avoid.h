
#ifndef OPENKAI_SRC_AUTOPILOT_ACTION_APMCOPTER_AVOID_H_
#define OPENKAI_SRC_AUTOPILOT_ACTION_APMCOPTER_AVOID_H_

#include "../../../Base/common.h"
#include "../../../Sensor/_Lightware_SF40.h"
#include "../../../Navigation/_Obstacle.h"
#include "../../../Stream/_ZED.h"
#include "../ActionBase.h"
#include "APMcopter_base.h"

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

public:
	void updateDistanceSensor(void);

private:
	APMcopter_base* m_pAPM;

	//sensor
	_Lightware_SF40* m_pSF40;
	_Obstacle* m_pObs;
	_ZED* m_pZED;
	vInt2		m_posMin;
	vDouble4	m_avoidArea;

	double	m_distObs;
	double	m_distSF40;

};

}

#endif

