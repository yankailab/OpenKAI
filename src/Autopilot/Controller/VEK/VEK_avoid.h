#ifndef OPENKAI_SRC_AUTOPILOT_ACTION_VEK_AVOID_H_
#define OPENKAI_SRC_AUTOPILOT_ACTION_VEK_AVOID_H_

#include "../../../Base/common.h"
#include "../../../Sensor/_Obstacle.h"
#include "../../ActionBase.h"
#include "VEK_base.h"

namespace kai
{

class VEK_avoid: public ActionBase
{
public:
	VEK_avoid();
	~VEK_avoid();

	bool init(void* pKiss);
	bool link(void);
	void update(void);
	bool draw(void);

private:
	VEK_base* m_pVEK;
	_Obstacle* m_pObs;

	double	m_vSteer;
	double m_distM;
	vDouble4 m_obsBox;
	vInt2 m_posMin;
	double m_alertDist;

};

}

#endif

