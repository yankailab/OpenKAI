
#ifndef OPENKAI_SRC_AUTOPILOT_ACTION_APMCOPTER_AVOID_H_
#define OPENKAI_SRC_AUTOPILOT_ACTION_APMCOPTER_AVOID_H_

#include "../../../Base/common.h"
#include "../../../Sensor/_Lightware_SF40.h"
#include "../../../Navigation/_Obstacle.h"
#include "../ActionBase.h"
#include "APMcopter_base.h"

#define N_DIR 38

namespace kai
{

struct DIR_DIST
{
	double m_rMin;
	double m_rMax;
	double m_dist;

	void init(void)
	{
		m_rMin = 0.0;
		m_rMax = DBL_MAX;
		m_dist = -1.0;
	}
};

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
	void updateZED(void);
	void updateSF40(void);
	void updateMavlink(void);

	APMcopter_base* m_pAPM;
	_Lightware_SF40* m_pSF40;
	_Obstacle* m_pObs;
	vInt2		m_posMin;
	vDouble4	m_avoidArea;

	DIR_DIST m_dd[N_DIR];

	double	m_distSF40;

};

}

#endif

